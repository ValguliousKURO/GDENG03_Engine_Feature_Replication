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
    // Don't destroy rigid body - PhysicsManager handles that
    m_rigidBody = nullptr;
    m_collider = nullptr;
}

void dx3d::PhysicsComponent::initialize()
{
    if (m_initialized) return;

    PhysicsManager::getInstance().registerComponent(this);
    createBody();
    m_initialized = true;
}

void dx3d::PhysicsComponent::onPhysicsDestroy()
{
    // Called by PhysicsManager before destroying the physics world
    m_rigidBody = nullptr;
    m_collider = nullptr;
    m_initialized = false;
}

void dx3d::PhysicsComponent::createBody()
{
    auto& physicsManager = PhysicsManager::getInstance();
    if (!physicsManager.isInitialized()) return;

    auto& transform = getGameObject().getTransform();
    Vec3 pos = transform.getPosition();
    Vec3 rot = transform.getRotation();
    Vec3 scale = transform.getScale();

    reactphysics3d::Vector3 position(pos.x, pos.y, pos.z);
    reactphysics3d::Quaternion orientation =
        reactphysics3d::Quaternion::fromEulerAngles(rot.x, rot.y, rot.z);

    m_rigidBody = physicsManager.createRigidBody(
        reactphysics3d::Transform(position, orientation));

    if (!m_rigidBody) return;

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

    // Handle mesh colliders
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

    // Create primitive collider
    reactphysics3d::CollisionShape* shape = nullptr;
    Vec3 scaledSize = { m_colliderSize.x * scale.x, m_colliderSize.y * scale.y, m_colliderSize.z * scale.z };

    switch (m_colliderType)
    {
    case PhysicsColliderType::Box:
        shape = physicsManager.createBoxShape({ scaledSize.x * 0.5f, scaledSize.y * 0.5f, scaledSize.z * 0.5f });
        break;
    case PhysicsColliderType::Sphere:
        shape = physicsManager.createSphereShape(scaledSize.x * 0.5f);
        break;
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
    if (!m_rigidBody || m_bodyType == PhysicsBodyType::Static) return;

    const auto& physicsTransform = m_rigidBody->getTransform();
    const auto& pos = physicsTransform.getPosition();
    const auto& orient = physicsTransform.getOrientation();

    auto& transform = getGameObject().getTransform();

    Vec3 newPos = { pos.x, pos.y, pos.z };

    float qx = orient.x, qy = orient.y, qz = orient.z, qw = orient.w;
    float sinr_cosp = 2.0f * (qw * qx + qy * qz);
    float cosr_cosp = 1.0f - 2.0f * (qx * qx + qy * qy);
    float roll = std::atan2(sinr_cosp, cosr_cosp);

    float sinp = 2.0f * (qw * qy - qz * qx);
    float pitch = (std::abs(sinp) >= 1.0f) ? std::copysign(3.14159265359f / 2.0f, sinp) : std::asin(sinp);

    float siny_cosp = 2.0f * (qw * qz + qx * qy);
    float cosy_cosp = 1.0f - 2.0f * (qy * qy + qz * qz);
    float yaw = std::atan2(siny_cosp, cosy_cosp);

    Vec3 newRot = { roll, pitch, yaw };

    transform.setPosition(newPos);
    transform.setRotation(newRot);
}

void dx3d::PhysicsComponent::syncTransformToPhysics()
{
    if (!m_rigidBody || m_bodyType == PhysicsBodyType::Static) return;

    auto& transform = getGameObject().getTransform();
    Vec3 pos = transform.getPosition();
    Vec3 rot = transform.getRotation();

    reactphysics3d::Vector3 position(pos.x, pos.y, pos.z);
    reactphysics3d::Quaternion orientation =
        reactphysics3d::Quaternion::fromEulerAngles(rot.x, rot.y, rot.z);

    m_rigidBody->setTransform(reactphysics3d::Transform(position, orientation));
}

void dx3d::PhysicsComponent::applyForce(const Vec3& force)
{
    if (m_rigidBody)
        m_rigidBody->applyWorldForceAtCenterOfMass({ force.x, force.y, force.z });
}

void dx3d::PhysicsComponent::applyImpulse(const Vec3& impulse)
{
    if (m_rigidBody && m_rigidBody->getType() == reactphysics3d::BodyType::DYNAMIC)
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
    if (m_rigidBody)
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

    // Convert vertices and apply scale
    std::vector<reactphysics3d::Vector3> rp3dVertices;
    rp3dVertices.reserve(m_meshVertices.size());

    for (const auto& v : m_meshVertices)
    {
        rp3dVertices.push_back(reactphysics3d::Vector3(
            v.x * scale.x, v.y * scale.y, v.z * scale.z));
    }

    // Create polygon faces
    reactphysics3d::PolygonVertexArray::PolygonFace* faces =
        new reactphysics3d::PolygonVertexArray::PolygonFace[m_meshIndices.size() / 3];

    for (size_t i = 0; i < m_meshIndices.size() / 3; i++)
    {
        faces[i].indexBase = static_cast<reactphysics3d::uint>(i * 3);
        faces[i].nbVertices = 3;
    }

    // Create PolygonVertexArray
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

    // Create ConvexMesh using PhysicsManager
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

    // Convert vertices and apply scale
    std::vector<reactphysics3d::Vector3> rp3dVertices;
    rp3dVertices.reserve(m_meshVertices.size());

    for (const auto& v : m_meshVertices)
    {
        rp3dVertices.push_back(reactphysics3d::Vector3(
            v.x * scale.x, v.y * scale.y, v.z * scale.z));
    }

    // Create TriangleVertexArray
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

    // Create TriangleMesh using PhysicsManager
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