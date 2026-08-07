#pragma once
#include <DX3D/Core/Base.h>
#include <reactphysics3d/reactphysics3d.h>
#include <vector>
#include <functional>

namespace dx3d
{
    class PhysicsComponent;
    class World;

    class PhysicsManager
    {
    public:
        static PhysicsManager& getInstance()
        {
            static PhysicsManager instance;
            return instance;
        }

        // Delete copy/move
        PhysicsManager(const PhysicsManager&) = delete;
        PhysicsManager& operator=(const PhysicsManager&) = delete;

        // Initialize/shutdown
        void initialize();
        void shutdown();

        // Frame update
        void update(float deltaTime);

        // Get physics objects
        reactphysics3d::PhysicsCommon* getPhysicsCommon() { return m_physicsCommon; }
        reactphysics3d::PhysicsWorld* getPhysicsWorld() { return m_physicsWorld; }

        // Register/unregister physics components
        void registerComponent(PhysicsComponent* component);
        void unregisterComponent(PhysicsComponent* component);

        // Create/destroy rigid bodies (managed lifecycle)
        reactphysics3d::RigidBody* createRigidBody(const reactphysics3d::Transform& transform);
        void destroyRigidBody(reactphysics3d::RigidBody* body);

        // Create collision shapes
        reactphysics3d::BoxShape* createBoxShape(const reactphysics3d::Vector3& halfExtents);
        reactphysics3d::SphereShape* createSphereShape(float radius);
        reactphysics3d::CapsuleShape* createCapsuleShape(float radius, float height);
        reactphysics3d::ConvexMeshShape* createConvexMeshShape(reactphysics3d::ConvexMesh* mesh);
        reactphysics3d::ConcaveMeshShape* createConcaveMeshShape(reactphysics3d::TriangleMesh* mesh);
        reactphysics3d::ConvexMesh* createConvexMesh(const reactphysics3d::PolygonVertexArray& vertexArray);
        reactphysics3d::TriangleMesh* createTriangleMesh(const reactphysics3d::TriangleVertexArray& vertexArray);

        bool isInitialized() const { return m_initialized; }

    private:
        PhysicsManager() = default;
        ~PhysicsManager();

        reactphysics3d::PhysicsCommon* m_physicsCommon = nullptr;
        reactphysics3d::PhysicsWorld* m_physicsWorld = nullptr;
        std::vector<PhysicsComponent*> m_components;
        bool m_initialized = false;
    };
}