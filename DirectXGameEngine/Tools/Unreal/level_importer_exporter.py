import json
import math
import os
import unreal


# Change this if you want a different size conversion.
# Unreal uses centimeters. This treats 1 editor/unity-style level unit as 100 Unreal units.
LEVEL_TO_UNREAL_SCALE = 100.0
UNREAL_TO_LEVEL_SCALE = 1.0 / LEVEL_TO_UNREAL_SCALE

# The C++ editor creates plane meshes with createPlaneMesh(10.0f, 10.0f).
# Unreal's /Engine/BasicShapes/Plane behaves like a 1x1 level-unit plane after
# the centimeter conversion. Therefore imported planes need 10x horizontal scale.
EDITOR_PLANE_BASE_SIZE = 10.0

# Match the current Unity handoff script behavior.
# Set to True only if you decide to store rotations in radians for cross-engine import/export.
ROTATION_VALUES_ARE_RADIANS = False

# Your C++ editor uses Y-up. Unreal uses Z-up.
# Level/editor vector: X=right, Y=up, Z=forward/depth
# Unreal vector:       X=right, Y=forward/depth, Z=up
REMAP_Y_UP_TO_UNREAL_Z_UP = True


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
            _apply_physics(actor, object_data)

        _move_player_starts_near_import(created_actors.values())

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
            root_component = _get_root_component(actor)
            if root_component:
                relative_transform = root_component.get_relative_transform()

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
            "scale": _unreal_scale_to_level(_transform_scale(relative_transform), _resolve_actor_type(actor)),
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
    root = _get_root_component(actor)
    if root:
        _set_component_mobility(root, unreal.ComponentMobility.MOVABLE)
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

    _set_actor_mobility(actor, unreal.ComponentMobility.MOVABLE)
    _set_actor_mobility(parent_actor, unreal.ComponentMobility.MOVABLE)

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
    scale = _level_scale_to_unreal(object_data.get("scale", [1.0, 1.0, 1.0]), object_data.get("type", ""))

    root = _get_root_component(actor)
    if root:
        _set_component_mobility(root, unreal.ComponentMobility.MOVABLE)
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


def _move_player_starts_near_import(imported_actors):
    imported_actors = [actor for actor in imported_actors if actor is not None and _get_static_mesh_component(actor) is not None]
    if not imported_actors:
        return

    center, extent, min_point, max_point = _calculate_actor_bounds(imported_actors)
    player_start_location = unreal.Vector(
        min_point.x + max(extent.x * 0.2, 250.0),
        min_point.y + max(extent.y * 0.2, 250.0),
        max(min_point.z + 120.0, 120.0),
    )
    direction_to_center = unreal.Vector(
        center.x - player_start_location.x,
        center.y - player_start_location.y,
        0.0,
    )
    yaw = math.degrees(math.atan2(direction_to_center.y, direction_to_center.x))
    player_start_location = unreal.Vector(
        min(max(player_start_location.x, min_point.x + 120.0), max_point.x - 120.0),
        min(max(player_start_location.y, min_point.y + 120.0), max_point.y - 120.0),
        player_start_location.z,
    )
    player_start_rotation = unreal.Rotator(0.0, yaw, 0.0)

    moved_count = 0
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if actor is None or "PlayerStart" not in actor.get_class().get_name():
            continue
        actor.set_actor_location(player_start_location, False, False)
        actor.set_actor_rotation(player_start_rotation, False)
        moved_count += 1

    if moved_count > 0:
        unreal.log("Moved {0} PlayerStart actor(s) near imported level.".format(moved_count))


def _calculate_actor_bounds(actors):
    first = True
    min_point = unreal.Vector(0.0, 0.0, 0.0)
    max_point = unreal.Vector(0.0, 0.0, 0.0)

    for actor in actors:
        origin, extent = actor.get_actor_bounds(False)
        actor_min = unreal.Vector(origin.x - extent.x, origin.y - extent.y, origin.z - extent.z)
        actor_max = unreal.Vector(origin.x + extent.x, origin.y + extent.y, origin.z + extent.z)

        if first:
            min_point = actor_min
            max_point = actor_max
            first = False
            continue

        min_point = unreal.Vector(
            min(min_point.x, actor_min.x),
            min(min_point.y, actor_min.y),
            min(min_point.z, actor_min.z),
        )
        max_point = unreal.Vector(
            max(max_point.x, actor_max.x),
            max(max_point.y, actor_max.y),
            max(max_point.z, actor_max.z),
        )

    center = unreal.Vector(
        (min_point.x + max_point.x) * 0.5,
        (min_point.y + max_point.y) * 0.5,
        (min_point.z + max_point.z) * 0.5,
    )
    extent = unreal.Vector(
        (max_point.x - min_point.x) * 0.5,
        (max_point.y - min_point.y) * 0.5,
        (max_point.z - min_point.z) * 0.5,
    )
    return center, extent, min_point, max_point


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


def _get_root_component(actor):
    if actor is None:
        return None
    try:
        return actor.get_root_component()
    except Exception:
        pass
    try:
        return actor.root_component
    except Exception:
        pass
    try:
        return actor.get_editor_property("root_component")
    except Exception:
        return None


def _set_actor_mobility(actor, mobility):
    root = _get_root_component(actor)
    if root:
        _set_component_mobility(root, mobility)


def _set_component_mobility(component, mobility):
    if component is None:
        return
    try:
        component.set_mobility(mobility)
    except Exception:
        try:
            component.set_editor_property("mobility", mobility)
        except Exception:
            pass


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

    should_simulate = physics_enabled and body_type == "Dynamic"
    _set_component_mobility(
        component,
        unreal.ComponentMobility.MOVABLE if should_simulate else unreal.ComponentMobility.STATIC,
    )
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    component.set_enable_gravity(bool(object_data.get("rigidBodyUseGravity", False)))

    mass = float(object_data.get("rigidBodyMass", 0.0))
    if mass > 0.0:
        component.set_mass_override_in_kg("", mass, True)

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
    return _unreal_size_to_level(size_local)


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


def _is_plane_type(object_type):
    return object_type in ("Plane", "Physics-Plane")


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
    if REMAP_Y_UP_TO_UNREAL_Z_UP:
        return unreal.Vector(
            vector.x * LEVEL_TO_UNREAL_SCALE,
            vector.z * LEVEL_TO_UNREAL_SCALE,
            vector.y * LEVEL_TO_UNREAL_SCALE,
        )
    return unreal.Vector(
        vector.x * LEVEL_TO_UNREAL_SCALE,
        vector.y * LEVEL_TO_UNREAL_SCALE,
        vector.z * LEVEL_TO_UNREAL_SCALE,
    )


def _unreal_vector_to_level(vector):
    if REMAP_Y_UP_TO_UNREAL_Z_UP:
        return [
            float(vector.x * UNREAL_TO_LEVEL_SCALE),
            float(vector.z * UNREAL_TO_LEVEL_SCALE),
            float(vector.y * UNREAL_TO_LEVEL_SCALE),
        ]
    return [
        float(vector.x * UNREAL_TO_LEVEL_SCALE),
        float(vector.y * UNREAL_TO_LEVEL_SCALE),
        float(vector.z * UNREAL_TO_LEVEL_SCALE),
    ]


def _unreal_vector_to_plain(vector):
    return [float(vector.x), float(vector.y), float(vector.z)]


def _level_scale_to_unreal(values, object_type=""):
    vector = _plain_vector(values)
    if REMAP_Y_UP_TO_UNREAL_Z_UP:
        scale = unreal.Vector(vector.x, vector.z, vector.y)
    else:
        scale = vector

    if _is_plane_type(object_type):
        scale.x *= EDITOR_PLANE_BASE_SIZE
        if REMAP_Y_UP_TO_UNREAL_Z_UP:
            scale.y *= EDITOR_PLANE_BASE_SIZE
        else:
            scale.z *= EDITOR_PLANE_BASE_SIZE

    return scale


def _unreal_scale_to_level(vector, object_type=""):
    adjusted = unreal.Vector(vector.x, vector.y, vector.z)
    if _is_plane_type(object_type):
        adjusted.x /= EDITOR_PLANE_BASE_SIZE
        if REMAP_Y_UP_TO_UNREAL_Z_UP:
            adjusted.y /= EDITOR_PLANE_BASE_SIZE
        else:
            adjusted.z /= EDITOR_PLANE_BASE_SIZE

    if REMAP_Y_UP_TO_UNREAL_Z_UP:
        return [float(adjusted.x), float(adjusted.z), float(adjusted.y)]
    return _unreal_vector_to_plain(adjusted)


def _unreal_size_to_level(vector):
    if REMAP_Y_UP_TO_UNREAL_Z_UP:
        return [
            float(vector.x * UNREAL_TO_LEVEL_SCALE),
            float(vector.z * UNREAL_TO_LEVEL_SCALE),
            float(vector.y * UNREAL_TO_LEVEL_SCALE),
        ]
    return _unreal_vector_to_level(vector)


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
