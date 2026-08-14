import json
import math
import os
import unreal


# Change this if you want a different size conversion.
# Unreal uses centimeters. This treats 1 editor/unity-style level unit as 100 Unreal units.
LEVEL_TO_UNREAL_SCALE = 100.0
UNREAL_TO_LEVEL_SCALE = 1.0 / LEVEL_TO_UNREAL_SCALE

# Match the current Unity handoff script behavior.
# Set to True only if you decide to store rotations in radians for cross-engine import/export.
ROTATION_VALUES_ARE_RADIANS = False


PRIMITIVE_MESH_PATHS = {
    "Plane": "/Engine/BasicShapes/Plane.Plane",
    "Cube": "/Engine/BasicShapes/Cube.Cube",
    "Physics-Cube": "/Engine/BasicShapes/Cube.Cube",
    "Sphere": "/Engine/BasicShapes/Sphere.Sphere",
    "Physics-Sphere": "/Engine/BasicShapes/Sphere.Sphere",
    "Capsule": "/Engine/BasicShapes/Capsule.Capsule",
    "Physics-Capsule": "/Engine/BasicShapes/Capsule.Capsule",
    "Cylinder": "/Engine/BasicShapes/Cylinder.Cylinder",
    "Physics-Cylinder": "/Engine/BasicShapes/Cylinder.Cylinder",
}


def import_level(level_path):
    """Import a .level file into the currently open Unreal level."""
    level_path = os.path.normpath(level_path)
    with open(level_path, "r", encoding="utf-8") as file:
        level_data = json.load(file)

    objects = level_data.get("objects", [])
    created_actors = {}

    with unreal.ScopedEditorTransaction("Import .level"):
        root_actor = _spawn_empty_actor(os.path.splitext(os.path.basename(level_path))[0])

        for object_data in objects:
            actor = _spawn_actor_for_object(object_data)
            if actor is None:
                continue

            object_id = int(object_data.get("id", 0))
            actor.set_actor_label(object_data.get("name") or object_data.get("type") or "GameObject")
            _set_actor_enabled(actor, bool(object_data.get("enabled", True)))
            _apply_point_light(actor, object_data)
            _apply_physics(actor, object_data)

            created_actors[object_id] = actor
            _attach_actor(actor, root_actor)

        for object_data in objects:
            object_id = int(object_data.get("id", 0))
            parent_id = int(object_data.get("parentId", 0))
            actor = created_actors.get(object_id)
            if actor is None:
                continue

            parent_actor = created_actors.get(parent_id, root_actor)
            _attach_actor(actor, parent_actor)
            _apply_relative_transform(actor, object_data)

    unreal.log("Imported {0} actors from {1}".format(len(created_actors), level_path))


def export_level(level_path):
    """Export the currently open Unreal level to the shared .level JSON schema."""
    level_path = os.path.normpath(level_path)
    actors = [_actor for _actor in unreal.EditorLevelLibrary.get_all_level_actors() if _should_export_actor(_actor)]
    actor_ids = {actor: index + 1 for index, actor in enumerate(actors)}

    objects = []
    for actor in actors:
        parent = actor.get_attach_parent_actor()
        component = _get_static_mesh_component(actor)
        point_light = _get_point_light_component(actor)
        relative_transform = actor.get_actor_transform()

        if parent in actor_ids:
            relative_transform = actor.get_root_component().get_relative_transform()

        object_data = {
            "id": actor_ids[actor],
            "parentId": actor_ids.get(parent, 0),
            "name": actor.get_actor_label(),
            "type": _resolve_actor_type(actor),
            "texture": "",
            "enabled": _is_actor_enabled(actor),
            "hasRigidBody": _has_rigid_body(component),
            "physicsEnabled": _has_rigid_body(component),
            "rigidBodyEnabled": _has_rigid_body(component),
            "rigidBodyType": _resolve_rigid_body_type(component),
            "rigidBodyCollider": _resolve_collider_type(actor, component),
            "rigidBodyColliderSize": _resolve_collider_size(actor, component),
            "rigidBodyMass": _resolve_mass(component),
            "rigidBodyUseGravity": bool(component and component.is_gravity_enabled()),
            "pointLightIntensity": float(point_light.intensity) if point_light else 0.0,
            "pointLightRange": float(point_light.attenuation_radius * UNREAL_TO_LEVEL_SCALE) if point_light else 0.0,
            "position": _unreal_vector_to_level(_transform_translation(relative_transform)),
            "rotation": _unreal_rotator_to_level(_transform_rotator(relative_transform)),
            "scale": _unreal_vector_to_plain(_transform_scale(relative_transform)),
        }
        objects.append(object_data)

    output = {
        "version": 1,
        "objects": objects,
    }

    os.makedirs(os.path.dirname(level_path), exist_ok=True)
    with open(level_path, "w", encoding="utf-8") as file:
        json.dump(output, file, indent=2)

    unreal.log("Exported {0} actors to {1}".format(len(objects), level_path))


def _spawn_actor_for_object(object_data):
    object_type = object_data.get("type", "")
    if object_type == "Point Light":
        return unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PointLight, unreal.Vector(0.0, 0.0, 0.0))

    mesh = _load_mesh_for_type(object_type)
    if mesh:
        return unreal.EditorLevelLibrary.spawn_actor_from_object(mesh, unreal.Vector(0.0, 0.0, 0.0))

    return _spawn_empty_actor(object_type or "GameObject")


def _spawn_empty_actor(label):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.Actor, unreal.Vector(0.0, 0.0, 0.0))
    actor.set_actor_label(label)
    return actor


def _load_mesh_for_type(object_type):
    if object_type.startswith("Obj:"):
        asset_name = object_type[len("Obj:"):]
        mesh = _find_static_mesh_by_name(asset_name)
        if mesh:
            return mesh
        unreal.log_warning("Could not find StaticMesh asset for {0}. Spawning empty actor.".format(object_type))
        return None

    mesh_path = PRIMITIVE_MESH_PATHS.get(object_type)
    if not mesh_path:
        return None

    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if mesh is None and "Capsule" in object_type:
        unreal.log_warning("Unreal BasicShapes capsule was not found. Spawning cylinder fallback for capsule.")
        mesh = unreal.EditorAssetLibrary.load_asset(PRIMITIVE_MESH_PATHS["Cylinder"])
    return mesh


def _find_static_mesh_by_name(asset_name):
    asset_name_lower = asset_name.lower()
    for asset_path in unreal.EditorAssetLibrary.list_assets("/Game", True, False):
        if os.path.splitext(os.path.basename(asset_path))[0].lower() != asset_name_lower:
            continue

        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if isinstance(asset, unreal.StaticMesh):
            return asset

    return None


def _attach_actor(actor, parent_actor):
    if actor is None or parent_actor is None or actor == parent_actor:
        return

    try:
        actor.attach_to_actor(
            parent_actor,
            "",
            unreal.AttachmentRule.KEEP_RELATIVE,
            unreal.AttachmentRule.KEEP_RELATIVE,
            unreal.AttachmentRule.KEEP_RELATIVE,
            False,
        )
    except TypeError:
        actor.attach_to_actor(parent_actor, "", unreal.AttachmentRule.KEEP_RELATIVE)


def _apply_relative_transform(actor, object_data):
    location = _level_vector_to_unreal(object_data.get("position", [0.0, 0.0, 0.0]))
    rotation = _level_rotation_to_unreal(object_data.get("rotation", [0.0, 0.0, 0.0]))
    scale = _plain_vector(object_data.get("scale", [1.0, 1.0, 1.0]))

    root = actor.get_root_component()
    if root:
        _set_component_relative_location(root, location)
        _set_component_relative_rotation(root, rotation)
        root.set_relative_scale3d(scale)
    else:
        actor.set_actor_location(location, False, False)
        actor.set_actor_rotation(rotation, False)
        actor.set_actor_scale3d(scale)


def _set_actor_enabled(actor, enabled):
    actor.set_actor_hidden_in_game(not enabled)
    actor.set_actor_enable_collision(enabled)
    try:
        actor.set_is_temporarily_hidden_in_editor(not enabled)
    except Exception:
        pass


def _is_actor_enabled(actor):
    hidden_in_editor = False
    hidden_in_game = False
    try:
        hidden_in_editor = actor.is_hidden_ed()
    except Exception:
        pass
    try:
        hidden_in_game = actor.is_hidden()
    except Exception:
        pass
    return not hidden_in_editor and not hidden_in_game


def _set_component_relative_location(component, location):
    try:
        component.set_relative_location(location, False, None, False)
    except TypeError:
        component.set_relative_location(location, False, False)


def _set_component_relative_rotation(component, rotation):
    try:
        component.set_relative_rotation(rotation, False, None, False)
    except TypeError:
        component.set_relative_rotation(rotation, False, False)


def _apply_point_light(actor, object_data):
    point_light = _get_point_light_component(actor)
    if not point_light:
        return

    intensity = float(object_data.get("pointLightIntensity", 0.0))
    light_range = float(object_data.get("pointLightRange", 0.0))
    if intensity > 0.0:
        point_light.set_intensity(intensity)
    if light_range > 0.0:
        point_light.set_attenuation_radius(light_range * LEVEL_TO_UNREAL_SCALE)


def _apply_physics(actor, object_data):
    component = _get_static_mesh_component(actor)
    if component is None:
        return

    has_rigid_body = bool(object_data.get("hasRigidBody", False) or object_data.get("physicsEnabled", False) or object_data.get("rigidBodyEnabled", False))
    if not has_rigid_body:
        component.set_simulate_physics(False)
        component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        return

    body_type = object_data.get("rigidBodyType", "")
    physics_enabled = bool(object_data.get("physicsEnabled", True) or object_data.get("rigidBodyEnabled", True))

    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    component.set_enable_gravity(bool(object_data.get("rigidBodyUseGravity", False)))

    mass = float(object_data.get("rigidBodyMass", 0.0))
    if mass > 0.0:
        component.set_mass_override_in_kg("", mass, True)

    should_simulate = physics_enabled and body_type == "Dynamic"
    component.set_simulate_physics(should_simulate)


def _get_static_mesh_component(actor):
    if actor is None:
        return None
    components = actor.get_components_by_class(unreal.StaticMeshComponent)
    return components[0] if components else None


def _get_point_light_component(actor):
    if actor is None:
        return None
    components = actor.get_components_by_class(unreal.PointLightComponent)
    return components[0] if components else None


def _has_rigid_body(component):
    if component is None:
        return False
    return component.is_simulating_physics() or component.get_collision_enabled() != unreal.CollisionEnabled.NO_COLLISION


def _resolve_rigid_body_type(component):
    if component is None or not _has_rigid_body(component):
        return ""
    return "Dynamic" if component.is_simulating_physics() else "Static"


def _resolve_collider_type(actor, component):
    actor_type = _resolve_actor_type(actor)
    if actor_type == "Sphere":
        return "Sphere"
    if actor_type in ("Capsule", "Cylinder"):
        return "Capsule"
    if component is not None and _has_rigid_body(component):
        return "Box"
    return ""


def _resolve_collider_size(actor, component):
    if component is None or not _has_rigid_body(component):
        return [0.0, 0.0, 0.0]

    bounds = component.bounds
    extent = bounds.box_extent
    actor_scale = actor.get_actor_scale3d()
    safe_scale = unreal.Vector(
        actor_scale.x if abs(actor_scale.x) > 0.0001 else 1.0,
        actor_scale.y if abs(actor_scale.y) > 0.0001 else 1.0,
        actor_scale.z if abs(actor_scale.z) > 0.0001 else 1.0,
    )

    size_world = unreal.Vector(extent.x * 2.0, extent.y * 2.0, extent.z * 2.0)
    size_local = unreal.Vector(
        size_world.x / safe_scale.x,
        size_world.y / safe_scale.y,
        size_world.z / safe_scale.z,
    )
    return _unreal_vector_to_level(size_local)


def _resolve_mass(component):
    if component is None or not _has_rigid_body(component):
        return 0.0
    try:
        return float(component.get_mass())
    except Exception:
        return 1.0


def _resolve_actor_type(actor):
    point_light = _get_point_light_component(actor)
    if point_light:
        return "Point Light"

    component = _get_static_mesh_component(actor)
    if component and component.static_mesh:
        mesh_name = component.static_mesh.get_name().lower()
        if "plane" in mesh_name:
            return "Plane"
        if "cube" in mesh_name:
            return "Cube"
        if "sphere" in mesh_name:
            return "Sphere"
        if "capsule" in mesh_name:
            return "Capsule"
        if "cylinder" in mesh_name:
            return "Cylinder"
        return "Obj:" + component.static_mesh.get_name()

    return "Empty"


def _should_export_actor(actor):
    if actor is None:
        return False

    class_name = actor.get_class().get_name()
    if class_name in ("WorldSettings", "Brush", "LevelScriptActor", "DefaultPhysicsVolume"):
        return False

    if actor.get_components_by_class(unreal.CameraComponent):
        return False

    return _get_static_mesh_component(actor) is not None or _get_point_light_component(actor) is not None


def _level_vector_to_unreal(values):
    vector = _plain_vector(values)
    return unreal.Vector(
        vector.x * LEVEL_TO_UNREAL_SCALE,
        vector.y * LEVEL_TO_UNREAL_SCALE,
        vector.z * LEVEL_TO_UNREAL_SCALE,
    )


def _unreal_vector_to_level(vector):
    return [
        float(vector.x * UNREAL_TO_LEVEL_SCALE),
        float(vector.y * UNREAL_TO_LEVEL_SCALE),
        float(vector.z * UNREAL_TO_LEVEL_SCALE),
    ]


def _unreal_vector_to_plain(vector):
    return [float(vector.x), float(vector.y), float(vector.z)]


def _plain_vector(values):
    if not isinstance(values, list) or len(values) < 3:
        return unreal.Vector(0.0, 0.0, 0.0)
    return unreal.Vector(float(values[0]), float(values[1]), float(values[2]))


def _transform_translation(transform):
    try:
        return transform.translation
    except Exception:
        return transform.get_translation()


def _transform_scale(transform):
    try:
        return transform.scale3d
    except Exception:
        return transform.get_scale3d()


def _transform_rotator(transform):
    try:
        return transform.rotation.rotator()
    except Exception:
        return transform.get_rotation().rotator()


def _level_rotation_to_unreal(values):
    vector = _plain_vector(values)
    if ROTATION_VALUES_ARE_RADIANS:
        vector = unreal.Vector(
            math.degrees(vector.x),
            math.degrees(vector.y),
            math.degrees(vector.z),
        )
    return unreal.Rotator(vector.x, vector.y, vector.z)


def _unreal_rotator_to_level(rotator):
    values = [float(rotator.roll), float(rotator.pitch), float(rotator.yaw)]
    if ROTATION_VALUES_ARE_RADIANS:
        values = [math.radians(value) for value in values]
    return values


# Usage from Unreal Python console:
# import level_importer_exporter
# level_importer_exporter.import_level(r"C:\path\to\scene.level")
# level_importer_exporter.export_level(r"C:\path\to\scene_from_unreal.level")
