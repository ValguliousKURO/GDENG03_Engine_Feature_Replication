#include <DX3D/Physics/PhysicsManager.h>
#include <DX3D/Component/PhysicsComponent.h>
#include <algorithm>


dx3d::PhysicsManager::~PhysicsManager()
{
    shutdown();
}

void dx3d::PhysicsManager::initialize()
{
    if (m_initialized) return;

    m_physicsCommon = new reactphysics3d::PhysicsCommon();

    reactphysics3d::PhysicsWorld::WorldSettings settings;
    settings.defaultVelocitySolverNbIterations = 20;
    settings.defaultPositionSolverNbIterations = 10;
    settings.gravity = reactphysics3d::Vector3(0, -9.81f, 0);

    m_physicsWorld = m_physicsCommon->createPhysicsWorld(settings);
    m_initialized = true;
}

void dx3d::PhysicsManager::shutdown()
{
    if (!m_initialized) return;

    // Clear all component references first
    for (auto* comp : m_components)
    {
        if (comp)
        {
            comp->onPhysicsDestroy();
        }
    }
    m_components.clear();

    // Destroy physics world (this cleans up all bodies, colliders, etc.)
    if (m_physicsWorld && m_physicsCommon)
    {
        m_physicsCommon->destroyPhysicsWorld(m_physicsWorld);
        m_physicsWorld = nullptr;
    }

    // Delete physics common
    delete m_physicsCommon;
    m_physicsCommon = nullptr;

    m_initialized = false;
}

void dx3d::PhysicsManager::update(float deltaTime)
{
    if (!m_initialized || !m_physicsWorld) return;

    m_physicsWorld->update(deltaTime);

    // Sync all registered components
    for (auto* comp : m_components)
    {
        if (comp)
        {
            comp->syncPhysicsToTransform();
        }
    }
}

void dx3d::PhysicsManager::registerComponent(PhysicsComponent* component)
{
    if (component && std::find(m_components.begin(), m_components.end(), component) == m_components.end())
    {
        m_components.push_back(component);
    }
}

void dx3d::PhysicsManager::unregisterComponent(PhysicsComponent* component)
{
    auto it = std::find(m_components.begin(), m_components.end(), component);
    if (it != m_components.end())
    {
        m_components.erase(it);
    }
}

reactphysics3d::RigidBody* dx3d::PhysicsManager::createRigidBody(const reactphysics3d::Transform& transform)
{
    if (!m_physicsWorld) return nullptr;
    return m_physicsWorld->createRigidBody(transform);
}

void dx3d::PhysicsManager::destroyRigidBody(reactphysics3d::RigidBody* body)
{
    if (m_physicsWorld && body)
    {
        m_physicsWorld->destroyRigidBody(body);
    }
}

reactphysics3d::BoxShape* dx3d::PhysicsManager::createBoxShape(const reactphysics3d::Vector3& halfExtents)
{
    if (!m_physicsCommon) return nullptr;
    return m_physicsCommon->createBoxShape(halfExtents);
}

reactphysics3d::SphereShape* dx3d::PhysicsManager::createSphereShape(float radius)
{
    if (!m_physicsCommon) return nullptr;
    return m_physicsCommon->createSphereShape(radius);
}

reactphysics3d::CapsuleShape* dx3d::PhysicsManager::createCapsuleShape(float radius, float height)
{
    if (!m_physicsCommon) return nullptr;
    return m_physicsCommon->createCapsuleShape(radius, height);
}

reactphysics3d::ConvexMeshShape* dx3d::PhysicsManager::createConvexMeshShape(reactphysics3d::ConvexMesh* mesh)
{
    if (!m_physicsCommon) return nullptr;
    return m_physicsCommon->createConvexMeshShape(mesh);
}

reactphysics3d::ConcaveMeshShape* dx3d::PhysicsManager::createConcaveMeshShape(reactphysics3d::TriangleMesh* mesh)
{
    if (!m_physicsCommon) return nullptr;
    return m_physicsCommon->createConcaveMeshShape(mesh);
}

reactphysics3d::ConvexMesh* dx3d::PhysicsManager::createConvexMesh(const reactphysics3d::PolygonVertexArray& vertexArray)
{
    if (!m_physicsCommon) return nullptr;
    std::vector<reactphysics3d::Message> messages;
    return m_physicsCommon->createConvexMesh(vertexArray, messages);
}

reactphysics3d::TriangleMesh* dx3d::PhysicsManager::createTriangleMesh(const reactphysics3d::TriangleVertexArray& vertexArray)
{
    if (!m_physicsCommon) return nullptr;
    std::vector<reactphysics3d::Message> messages;
    return m_physicsCommon->createTriangleMesh(vertexArray, messages);
}
