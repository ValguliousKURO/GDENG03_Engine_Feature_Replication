#pragma once
#include <DX3D/Game/Component.h>
#include <DX3D/Math/Vec3.h>
#include <reactphysics3d/reactphysics3d.h>
#include <vector>
#include <unordered_map>

namespace dx3d
{
    enum class PhysicsBodyType { Static, Dynamic, Kinematic };
    enum class PhysicsColliderType { Box, Sphere, Capsule, ConvexMesh, ConcaveMesh };

    class PhysicsComponent final : public Component
    {
        dx3d_typeid(PhysicsComponent)
    public:
        PhysicsComponent(const ComponentDesc& desc);
        ~PhysicsComponent();

        void initialize();
        void syncPhysicsToTransform();
        void syncTransformToPhysics();
        void onPhysicsDestroy();  // Called when physics manager shuts down

        // Scene lifecycle
        void onStart();
        void onEnd();

        // Setters
        void setBodyType(PhysicsBodyType type);
        void setColliderType(PhysicsColliderType type) { m_colliderType = type; }
        void setColliderSize(const Vec3& size) { m_colliderSize = size; }
        void setMass(float mass);
        void setUseGravity(bool use);
        void setPhysicsEnabled(bool enabled);
        void setMeshColliderData(const std::vector<Vec3>& vertices, const std::vector<ui32>& indices);

        // Forces
        void applyForce(const Vec3& force);
        void applyImpulse(const Vec3& impulse);
        void applyTorque(const Vec3& torque);

        // Getters
        PhysicsBodyType getBodyType() const { return m_bodyType; }
        PhysicsColliderType getColliderType() const { return m_colliderType; }
        Vec3 getColliderSize() const { return m_colliderSize; }
        float getMass() const { return m_mass; }
        bool isPhysicsEnabled() const { return m_physicsEnabled; }
        bool isUseGravityEnabled() const { return m_useGravity; }
        reactphysics3d::RigidBody* getRigidBody() const { return m_rigidBody; }

    private:
        void createBody();
        void createConvexMeshCollider();
        void createConcaveMeshCollider();

        PhysicsBodyType m_bodyType = PhysicsBodyType::Dynamic;
        PhysicsColliderType m_colliderType = PhysicsColliderType::Box;
        Vec3 m_colliderSize = { 1.0f, 1.0f, 1.0f };
        float m_mass = 1.0f;
        bool m_useGravity = true;
        bool m_physicsEnabled = true;

        std::vector<Vec3> m_meshVertices;
        std::vector<ui32> m_meshIndices;
        bool m_hasMeshData = false;

        reactphysics3d::RigidBody* m_rigidBody = nullptr;
        reactphysics3d::Collider* m_collider = nullptr;
        bool m_initialized = false;

        Vec3 m_previousPosition;
        Vec3 m_previousRotation;
        Vec3 m_previousScale;

        std::unordered_map<std::string, Vec3> m_initial_transform{};
    };
}
