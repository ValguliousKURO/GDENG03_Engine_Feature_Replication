#include<DX3D/Game/GameObject.h>
#include <DX3D/Game/Component.h>
#include <DX3D/Component/TransformComponent.h>
#include <DX3D/Game/World.h>
#include <cmath>
#include <algorithm>
#include <Windows.h>

dx3d::GameObject::GameObject(const GameObjectDesc& desc) : 
	Identifiable(desc.base),
	m_world(desc.world), 
	m_gameContext(desc.gameContext),
	m_windowInput(desc.windowInput)
{
	m_transform = createOrGetComponent<TransformComponent>();
}

dx3d::TransformComponent& dx3d::GameObject::getTransform() noexcept
{
	return *m_transform;
}

dx3d::Component* dx3d::GameObject::createComponentInternal(UniquePtr<Component>& component)
{
	if (!component) return {};
	auto typeId = component->getTypeId();
	auto ptr = component.get();
	if (m_components.find(typeId) != m_components.end()) return {};
	m_components.emplace(typeId, std::move(component));
	m_world.addComponentInternal(*ptr);
	return ptr;
}

dx3d::World& dx3d::GameObject::getWorld() noexcept
{
	return m_world;
}

dx3d::InputSystem& dx3d::GameObject::getInputSystem() noexcept
{
	// Prefer window‑specific input if available
	if (m_windowInput) return *m_windowInput;
	return m_gameContext.input;
}

dx3d::ResourceManager& dx3d::GameObject::getResourceManager() noexcept
{
	return m_gameContext.resourceManager;
}

void dx3d::GameObject::setName(std::string name)
{
	m_name = name;
}

void dx3d::GameObject::setParent(GameObject* newParent)
{
    if (m_parent == newParent || newParent == this) return;
    if (newParent && !newParent->isDeleted() && newParent->isDescendantOf(this)) return;

    // Get current WORLD transform values (force update if needed)
    Vec3 worldPos = getTransform().getWorldPosition();
    Vec3 worldRot = getTransform().getWorldRotation();
    Vec3 worldScale = getTransform().getWorldScale();

    // Unlink from current parent
    if (m_parent)
    {
        std::erase(m_parent->m_children, this);
    }

    // Assign new parent
    m_parent = (newParent && !newParent->isDeleted()) ? newParent : nullptr;

    if (m_parent)
    {
        m_parent->m_children.push_back(this);

        // Get parent's WORLD transform values
        Vec3 parentWorldPos = m_parent->getTransform().getWorldPosition();
        Vec3 parentWorldRot = m_parent->getTransform().getWorldRotation();
        Vec3 parentWorldScale = m_parent->getTransform().getWorldScale();

        // Calculate new LOCAL transform values
        Vec3 localPos = worldPos - parentWorldPos;
        Vec3 localRot = worldRot - parentWorldRot;
        Vec3 localScale;
        localScale.x = worldScale.x / parentWorldScale.x;
        localScale.y = worldScale.y / parentWorldScale.y;
        localScale.z = worldScale.z / parentWorldScale.z;

        // Set local transform
        getTransform().setPosition(localPos);
        getTransform().setRotation(localRot);
        getTransform().setScale(localScale);
    }
    else
    {
        // Unparenting: world becomes local
        getTransform().setPosition(worldPos);
        getTransform().setRotation(worldRot);
        getTransform().setScale(worldScale);
    }
}

bool dx3d::GameObject::isDescendantOf(const GameObject * potentialAncestor) const
{
	if (!potentialAncestor) return false;
	const GameObject* current = m_parent;
	while (current)
	{
		if (current == potentialAncestor) return true;
		current = current->m_parent;
	}
	return false;
}

bool dx3d::GameObject::isActiveInHierarchy() const noexcept
{
	//If the object itself is disabled or deleted, return false
	if (!m_enabled || m_deleted) return false;

	//If the object has a parent, recursively check if the parent is active in hierarchy
	if (m_parent) return m_parent->isActiveInHierarchy();

	return true;
}

dx3d::Component* dx3d::GameObject::getComponentInternal(size_t id)
{
	auto it = m_components.find(id);
	if (it == m_components.end()) return {};
	return it->second.get();
}

bool dx3d::GameObject::removeComponentInternal(size_t id)
{
	if (m_transform && id == m_transform->getTypeId()) return false;

	auto it = m_components.find(id);
	if (it == m_components.end()) return false;

	if (it->second)
	{
		m_world.removeComponentInternal(*it->second);
	}
	m_components.erase(it);
	return true;
}
