#include <DX3D/Component/PhysicsComponent.h>
#include <DX3D/Physics/PhysicsManager.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Component/TransformComponent.h>

dx3d::PhysicsComponent::PhysicsComponent(const ComponentDesc& desc)
    : Component(desc)
{}

dx3d::PhysicsComponent::~PhysicsComponent()
{
    PhysicsManager::getInstance().unregisterComponent(this);
    m_rigidBody = nullptr;
    m_collider = nullptr;
}

void dx3d::PhysicsComponent::initialize()
{
    if (m_initialized || !m_physicsEnabled) return;

    createBody();
    if (m_rigidBody)
    {
        PhysicsManager::getInstance().registerComponent(this);
        m_initialized = true;
    }
}

void dx3d::PhysicsComponent::onPhysicsDestroy()
{
    m_rigidBody = nullptr;
    m_collider = nullptr;
    m_initialized = false;
}

void dx3d::PhysicsComponent::onStart()
{
    if (!m_physicsEnabled) return;

    auto& transformComponent = getGameObject().getTransform();

    m_initial_transform["Position"] = transformComponent.getPosition();
    m_initial_transform["Scale"] = transformComponent.getScale();
    m_initial_transform["Rotation"] = transformComponent.getRotation();
}

void dx3d::PhysicsComponent::onEnd()
{
    if (!m_physicsEnabled) return;

    auto& transformComponent = getGameObject().getTransform();

    //Reset transform to initial values
    transformComponent.setPosition(m_initial_transform["Position"]);
    transformComponent.setScale(m_initial_transform["Scale"]);
    transformComponent.setRotation(m_initial_transform["Rotation"]);

    //Fully reset the physics body
    if (m_rigidBody)
    {
        //Reset velocity and forces
        m_rigidBody->setLinearVelocity(reactphysics3d::Vector3(0, 0, 0));
        m_rigidBody->setAngularVelocity(reactphysics3d::Vector3(0, 0, 0));

        //Reset transform to initial position
        Vec3 initPos = m_initial_transform["Position"];
        Vec3 initRot = m_initial_transform["Rotation"];

        //Use World position for the rigid body
        auto& transform = getGameObject().getTransform();
        transform.updateWorldMatrix();
        Mat4x4 worldMat = transform.getAffineWorldMatrix();
        Vec3 worldPos = { worldMat.row(3).x, worldMat.row(3).y, worldMat.row(3).z };

        reactphysics3d::Vector3 position(worldPos.x, worldPos.y, worldPos.z);
        reactphysics3d::Quaternion orientation =
            reactphysics3d::Quaternion::fromEulerAngles(initRot.x, initRot.y, initRot.z);
        m_rigidBody->setTransform(reactphysics3d::Transform(position, orientation));

        //Reset force and torque accumulators
        m_rigidBody->applyWorldForceAtCenterOfMass(reactphysics3d::Vector3(0, 0, 0));
        m_rigidBody->applyWorldTorque(reactphysics3d::Vector3(0, 0, 0));

        //Make sure the body is awake
        m_rigidBody->setIsSleeping(false);
    }

    // Reset previous tracking values
    m_previousPosition = m_initial_transform["Position"];
    m_previousRotation = m_initial_transform["Rotation"];
    m_previousScale = m_initial_transform["Scale"];
}

void dx3d::PhysicsComponent::createBody()
{
    if (!m_physicsEnabled || m_rigidBody) return;

    auto& physicsManager = PhysicsManager::getInstance();
    if (!physicsManager.isInitialized()) return;

    auto& transform = getGameObject().getTransform();

    //Force world matrix update to get correct world position
    transform.updateWorldMatrix();
    Mat4x4 worldMat = transform.getAffineWorldMatrix();

    //Extract world position from the world matrix
    Vec3 worldPos = { worldMat.row(3).x, worldMat.row(3).y, worldMat.row(3).z };

    //Extract world rotation from the world matrix
    Vec3 wr0 = { worldMat.row(0).x, worldMat.row(0).y, worldMat.row(0).z };
    Vec3 wr1 = { worldMat.row(1).x, worldMat.row(1).y, worldMat.row(1).z };
    Vec3 wr2 = { worldMat.row(2).x, worldMat.row(2).y, worldMat.row(2).z };

    float wr0Len = wr0.length();
    float wr1Len = wr1.length();
    float wr2Len = wr2.length();

    if (wr0Len > 0.00001f) wr0 = wr0 / wr0Len;
    if (wr1Len > 0.00001f) wr1 = wr1 / wr1Len;
    if (wr2Len > 0.00001f) wr2 = wr2 / wr2Len;

    wr0 = Vec3::normalize(wr0);
    wr1 = Vec3::normalize(wr1 - wr0 * Vec3::dot(wr0, wr1));
    wr2 = Vec3::cross(wr0, wr1);

    //Extract world rotation from orthonormal rows
    float sy = std::clamp(-wr0.z, -0.999999f, 0.999999f);
    float cy = std::sqrt(1.0f - sy * sy);
    float wy = std::atan2(sy, cy);
    float wx, wz;
    if (cy > 0.00001f) {
        wx = std::atan2(wr1.z, wr2.z);
        wz = std::atan2(wr0.y, wr0.x);
    }
    else {
        wx = 0.0f;
        wz = std::atan2(-wr1.x, wr1.y);
    }
    Vec3 worldRot = { wx, wy, wz };

    //Store initial transform for change detection
    m_previousPosition = worldPos;
    m_previousRotation = worldRot;
    m_previousScale = transform.getScale(); //Local scale for collider sizing

    reactphysics3d::Vector3 position(worldPos.x, worldPos.y, worldPos.z);
    reactphysics3d::Quaternion orientation =
        reactphysics3d::Quaternion::fromEulerAngles(worldRot.x, worldRot.y, worldRot.z);

    m_rigidBody = physicsManager.createRigidBody(
        reactphysics3d::Transform(position, orientation));

    if (!m_rigidBody) return;

    //Set body type
    reactphysics3d::BodyType bodyType;
    switch (m_bodyType)
    {
    case PhysicsBodyType::Static:
        bodyType = reactphysics3d::BodyType::STATIC;
        break;
    case PhysicsBodyType::Kinematic:
        bodyType = reactphysics3d::BodyType::KINEMATIC;
        break;
    default:
        bodyType = reactphysics3d::BodyType::DYNAMIC;
        break;
    }
    m_rigidBody->setType(bodyType);

    if (bodyType == reactphysics3d::BodyType::DYNAMIC)
    {
        m_rigidBody->enableGravity(m_useGravity);
        m_rigidBody->setMass(m_mass);
    }

    //Handle mesh colliders
    if (m_colliderType == PhysicsColliderType::ConvexMesh && m_hasMeshData)
    {
        createConvexMeshCollider();
        return;
    }
    if (m_colliderType == PhysicsColliderType::ConcaveMesh && m_hasMeshData)
    {
        createConcaveMeshCollider();
        return;
    }

    //Create primitive collider using local scale
    Vec3 localScale = transform.getScale();
    Vec3 scaledSize = {
        m_colliderSize.x * localScale.x,
        m_colliderSize.y * localScale.y,
        m_colliderSize.z * localScale.z
    };

    reactphysics3d::CollisionShape* shape = nullptr;
    switch (m_colliderType)
    {
    case PhysicsColliderType::Box:
        shape = physicsManager.createBoxShape({
            scaledSize.x * 0.5f,
            scaledSize.y * 0.5f,
            scaledSize.z * 0.5f
            });
        break;
    case PhysicsColliderType::Sphere:
    {
        float radius = std::max({ scaledSize.x, scaledSize.y, scaledSize.z }) * 0.5f;
        shape = physicsManager.createSphereShape(radius);
        break;
    }
    case PhysicsColliderType::Capsule:
        shape = physicsManager.createCapsuleShape(scaledSize.x * 0.5f, scaledSize.y);
        break;
    default:
        break;
    }

    if (shape && m_rigidBody)
    {
        m_collider = m_rigidBody->addCollider(shape, reactphysics3d::Transform::identity());
    }
}

void dx3d::PhysicsComponent::syncPhysicsToTransform()
{
    if (!m_physicsEnabled) return;
    if (!m_rigidBody) return;

	//Get the world transform of the game object
    auto& transform = getGameObject().getTransform();
    transform.updateWorldMatrix();
    Mat4x4 worldMat = transform.getAffineWorldMatrix();

    Vec3 worldPos = { worldMat.row(3).x, worldMat.row(3).y, worldMat.row(3).z };
    Vec3 localScale = transform.getScale();

	//Collider rescaling if the local scale has changed significantly
    bool scaleChanged = (std::abs(localScale.x - m_previousScale.x) > 0.0001f ||
        std::abs(localScale.y - m_previousScale.y) > 0.0001f ||
        std::abs(localScale.z - m_previousScale.z) > 0.0001f);

    if (scaleChanged && m_rigidBody &&
        m_colliderType != PhysicsColliderType::ConvexMesh &&
        m_colliderType != PhysicsColliderType::ConcaveMesh)
    {
        //Remove old collider
        if (m_collider)
        {
            m_rigidBody->removeCollider(m_collider);
            m_collider = nullptr;
        }

        //Create new collider with updated scale
        auto& physicsManager = PhysicsManager::getInstance();
        Vec3 scaledSize = {
            m_colliderSize.x * localScale.x,
            m_colliderSize.y * localScale.y,
            m_colliderSize.z * localScale.z
        };

        reactphysics3d::CollisionShape* shape = nullptr;
        switch (m_colliderType)
        {
        case PhysicsColliderType::Box:
            shape = physicsManager.createBoxShape({
                scaledSize.x * 0.5f,
                scaledSize.y * 0.5f,
                scaledSize.z * 0.5f
                });
            break;
        case PhysicsColliderType::Sphere:
        {
            float radius = std::max({ scaledSize.x, scaledSize.y, scaledSize.z }) * 0.5f;
            shape = physicsManager.createSphereShape(radius);
            break;
        }
        case PhysicsColliderType::Capsule:
            shape = physicsManager.createCapsuleShape(scaledSize.x * 0.5f, scaledSize.y);
            break;
        default:
            break;
        }

        if (shape)
        {
            m_collider = m_rigidBody->addCollider(shape, reactphysics3d::Transform::identity());
        }

        m_previousScale = localScale;
    }

	//Static bodies: Only sync from game to physics if the position has changed significantly
    if (m_bodyType == PhysicsBodyType::Static)
    {
        bool posChanged = (std::abs(worldPos.x - m_previousPosition.x) > 0.0001f ||
            std::abs(worldPos.y - m_previousPosition.y) > 0.0001f ||
            std::abs(worldPos.z - m_previousPosition.z) > 0.0001f);

        if (posChanged)
        {
            reactphysics3d::Vector3 position(worldPos.x, worldPos.y, worldPos.z);
            reactphysics3d::Quaternion orientation = m_rigidBody->getTransform().getOrientation();
            m_rigidBody->setTransform(reactphysics3d::Transform(position, orientation));
            m_previousPosition = worldPos;
        }
        return;
    }

	//Dynamic/Kinematic bodies: Only sync from game to physics if the position has changed significantly
    bool posChanged = (std::abs(worldPos.x - m_previousPosition.x) > 0.0001f ||
        std::abs(worldPos.y - m_previousPosition.y) > 0.0001f ||
        std::abs(worldPos.z - m_previousPosition.z) > 0.0001f);

    if (posChanged)
    {
        //Teleport physics body to world position
        reactphysics3d::Vector3 position(worldPos.x, worldPos.y, worldPos.z);
        reactphysics3d::Quaternion orientation = m_rigidBody->getTransform().getOrientation();
        m_rigidBody->setTransform(reactphysics3d::Transform(position, orientation));

        if (m_rigidBody->getType() == reactphysics3d::BodyType::DYNAMIC)
        {
            m_rigidBody->setLinearVelocity(reactphysics3d::Vector3(0, 0, 0));
            m_rigidBody->setAngularVelocity(reactphysics3d::Vector3(0, 0, 0));
        }

        m_previousPosition = worldPos;
        return;
    }

    //Physics to Transform Sync Update the Game Object's transform based on the physics body's transform
    const auto& physicsTransform = m_rigidBody->getTransform();
    const auto& pos = physicsTransform.getPosition();
    const auto& orient = physicsTransform.getOrientation();

    Vec3 newWorldPos = { pos.x, pos.y, pos.z };

    //Convert quaternion to Euler angles
    float qx = orient.x, qy = orient.y, qz = orient.z, qw = orient.w;
    float sinr_cosp = 2.0f * (qw * qx + qy * qz);
    float cosr_cosp = 1.0f - 2.0f * (qx * qx + qy * qy);
    float roll = std::atan2(sinr_cosp, cosr_cosp);
    float sinp = 2.0f * (qw * qy - qz * qx);
    float pitch = (std::abs(sinp) >= 1.0f) ? std::copysign(3.14159265359f / 2.0f, sinp) : std::asin(sinp);
    float siny_cosp = 2.0f * (qw * qz + qx * qy);
    float cosy_cosp = 1.0f - 2.0f * (qy * qy + qz * qz);
    float yaw = std::atan2(siny_cosp, cosy_cosp);
    Vec3 newWorldRot = { roll, pitch, yaw };

    //Convert world position/rotation back to local if the object has a parent
    GameObject* parent = getGameObject().getParent();
    if (parent)
    {
        auto& parentTransform = parent->getTransform();
        Vec3 parentWorldPos = parentTransform.getWorldPosition();
        Vec3 parentWorldRot = parentTransform.getWorldRotation();

        Vec3 localPos = newWorldPos - parentWorldPos;
        Vec3 localRot = newWorldRot - parentWorldRot;

        transform.setPosition(localPos);
        transform.setRotation(localRot);
    }
    else
    {
        transform.setPosition(newWorldPos);
        transform.setRotation(newWorldRot);
    }

    m_previousPosition = newWorldPos;
    m_previousRotation = newWorldRot;
}

void dx3d::PhysicsComponent::syncTransformToPhysics()
{
    if (!m_physicsEnabled) return;
    if (!m_rigidBody || m_bodyType == PhysicsBodyType::Static) return;

    auto& transform = getGameObject().getTransform();
    transform.updateWorldMatrix();
    Mat4x4 worldMat = transform.getAffineWorldMatrix();

    Vec3 worldPos = { worldMat.row(3).x, worldMat.row(3).y, worldMat.row(3).z };

    //Extract world rotation
    Vec3 wr0 = { worldMat.row(0).x, worldMat.row(0).y, worldMat.row(0).z };
    Vec3 wr1 = { worldMat.row(1).x, worldMat.row(1).y, worldMat.row(1).z };
    Vec3 wr2 = { worldMat.row(2).x, worldMat.row(2).y, worldMat.row(2).z };

    float wr0Len = wr0.length();
    float wr1Len = wr1.length();
    float wr2Len = wr2.length();

    if (wr0Len > 0.00001f) wr0 = wr0 / wr0Len;
    if (wr1Len > 0.00001f) wr1 = wr1 / wr1Len;
    if (wr2Len > 0.00001f) wr2 = wr2 / wr2Len;

    wr0 = Vec3::normalize(wr0);
    wr1 = Vec3::normalize(wr1 - wr0 * Vec3::dot(wr0, wr1));
    wr2 = Vec3::cross(wr0, wr1);

    float sy = std::clamp(-wr0.z, -0.999999f, 0.999999f);
    float cy = std::sqrt(1.0f - sy * sy);
    float wy = std::atan2(sy, cy);
    float wx, wz;
    if (cy > 0.00001f) {
        wx = std::atan2(wr1.z, wr2.z);
        wz = std::atan2(wr0.y, wr0.x);
    }
    else {
        wx = 0.0f;
        wz = std::atan2(-wr1.x, wr1.y);
    }
    Vec3 worldRot = { wx, wy, wz };

    reactphysics3d::Vector3 position(worldPos.x, worldPos.y, worldPos.z);
    reactphysics3d::Quaternion orientation =
        reactphysics3d::Quaternion::fromEulerAngles(worldRot.x, worldRot.y, worldRot.z);

    m_rigidBody->setTransform(reactphysics3d::Transform(position, orientation));

    m_previousPosition = worldPos;
    m_previousRotation = worldRot;
}

void dx3d::PhysicsComponent::applyForce(const Vec3& force)
{
    if (m_physicsEnabled && m_rigidBody)
        m_rigidBody->applyWorldForceAtCenterOfMass({ force.x, force.y, force.z });
}

void dx3d::PhysicsComponent::applyImpulse(const Vec3& impulse)
{
    if (m_physicsEnabled && m_rigidBody && m_rigidBody->getType() == reactphysics3d::BodyType::DYNAMIC)
    {
        float mass = m_rigidBody->getMass();
        if (mass > 0.0f)
        {
            auto currentVel = m_rigidBody->getLinearVelocity();
            m_rigidBody->setLinearVelocity(currentVel + reactphysics3d::Vector3(impulse.x, impulse.y, impulse.z) / mass);
        }
    }
}

void dx3d::PhysicsComponent::applyTorque(const Vec3& torque)
{
    if (m_physicsEnabled && m_rigidBody)
        m_rigidBody->applyWorldTorque({ torque.x, torque.y, torque.z });
}

void dx3d::PhysicsComponent::setBodyType(PhysicsBodyType type)
{
    m_bodyType = type;
    if (m_rigidBody)
    {
        switch (type)
        {
        case PhysicsBodyType::Static:
            m_rigidBody->setType(reactphysics3d::BodyType::STATIC);
            break;
        case PhysicsBodyType::Kinematic:
            m_rigidBody->setType(reactphysics3d::BodyType::KINEMATIC);
            break;
        default:
            m_rigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
            break;
        }
    }
}

void dx3d::PhysicsComponent::setMass(float mass)
{
    m_mass = mass;
    if (m_rigidBody)
        m_rigidBody->setMass(mass);
}

void dx3d::PhysicsComponent::setUseGravity(bool use)
{
    m_useGravity = use;
    if (m_rigidBody)
        m_rigidBody->enableGravity(use);
}

void dx3d::PhysicsComponent::setPhysicsEnabled(bool enabled)
{
    if (m_physicsEnabled == enabled) return;

    if (!enabled)
    {
        syncPhysicsToTransform();
        m_physicsEnabled = false;
        PhysicsManager::getInstance().unregisterComponent(this);
        PhysicsManager::getInstance().destroyRigidBody(m_rigidBody);
        m_rigidBody = nullptr;
        m_collider = nullptr;
        m_initialized = false;
        return;
    }

    m_physicsEnabled = true;
    initialize();
    syncTransformToPhysics();
}

void dx3d::PhysicsComponent::setMeshColliderData(const std::vector<Vec3>& vertices, const std::vector<ui32>& indices)
{
    m_meshVertices = vertices;
    m_meshIndices = indices;
    m_hasMeshData = true;
}

void dx3d::PhysicsComponent::createConvexMeshCollider()
{
    auto& physicsManager = PhysicsManager::getInstance();
    if (!physicsManager.isInitialized() || !m_hasMeshData) return;

    auto& transform = getGameObject().getTransform();
    Vec3 scale = transform.getScale();

    std::vector<reactphysics3d::Vector3> rp3dVertices;
    rp3dVertices.reserve(m_meshVertices.size());

    for (const auto& v : m_meshVertices)
    {
        rp3dVertices.push_back(reactphysics3d::Vector3(
            v.x * scale.x, v.y * scale.y, v.z * scale.z));
    }

    reactphysics3d::PolygonVertexArray::PolygonFace* faces =
        new reactphysics3d::PolygonVertexArray::PolygonFace[m_meshIndices.size() / 3];

    for (size_t i = 0; i < m_meshIndices.size() / 3; i++)
    {
        faces[i].indexBase = static_cast<reactphysics3d::uint>(i * 3);
        faces[i].nbVertices = 3;
    }

    reactphysics3d::PolygonVertexArray polygonVertexArray(
        static_cast<reactphysics3d::uint>(rp3dVertices.size()),
        rp3dVertices.data(),
        sizeof(reactphysics3d::Vector3),
        m_meshIndices.data(),
        sizeof(ui32),
        static_cast<reactphysics3d::uint>(m_meshIndices.size() / 3),
        faces,
        reactphysics3d::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
        reactphysics3d::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE
    );

    reactphysics3d::ConvexMesh* convexMesh = physicsManager.createConvexMesh(polygonVertexArray);

    if (convexMesh && m_rigidBody)
    {
        reactphysics3d::ConvexMeshShape* convexShape = physicsManager.createConvexMeshShape(convexMesh);

        if (convexShape)
        {
            m_collider = m_rigidBody->addCollider(
                convexShape, reactphysics3d::Transform::identity());
        }
    }

    delete[] faces;
}

void dx3d::PhysicsComponent::createConcaveMeshCollider()
{
    auto& physicsManager = PhysicsManager::getInstance();
    if (!physicsManager.isInitialized() || !m_hasMeshData) return;

    auto& transform = getGameObject().getTransform();
    Vec3 scale = transform.getScale();

    std::vector<reactphysics3d::Vector3> rp3dVertices;
    rp3dVertices.reserve(m_meshVertices.size());

    for (const auto& v : m_meshVertices)
    {
        rp3dVertices.push_back(reactphysics3d::Vector3(
            v.x * scale.x, v.y * scale.y, v.z * scale.z));
    }

    reactphysics3d::TriangleVertexArray triangleVertexArray(
        static_cast<reactphysics3d::uint>(rp3dVertices.size()),
        rp3dVertices.data(),
        sizeof(reactphysics3d::Vector3),
        static_cast<reactphysics3d::uint>(m_meshIndices.size() / 3),
        m_meshIndices.data(),
        sizeof(ui32) * 3,
        reactphysics3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
        reactphysics3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE
    );

    reactphysics3d::TriangleMesh* triangleMesh = physicsManager.createTriangleMesh(triangleVertexArray);

    if (triangleMesh && m_rigidBody)
    {
        reactphysics3d::ConcaveMeshShape* concaveShape = physicsManager.createConcaveMeshShape(triangleMesh);

        if (concaveShape)
        {
            m_collider = m_rigidBody->addCollider(
                concaveShape, reactphysics3d::Transform::identity());
        }
    }
}