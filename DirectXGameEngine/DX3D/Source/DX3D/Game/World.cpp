#include <DX3D/Game/World.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Game/Component.h>
#include <DX3D/Component/TransformComponent.h>

dx3d::World::World(const WorldDesc& desc) : 
    Base(desc.base), 
    m_gameContext(desc.gameContext)
{
}

void dx3d::World::update(f32 deltaTime)
{
    if (m_events.size())
    {
        std::swap(m_events, m_eventsSwapBuffer);

        for (auto& e : m_eventsSwapBuffer)
        {
            if (e.eventType == EventType::Create)
            {
                e.object->onCreate();  // Call onCreate on the object
            }
        }

        m_eventsSwapBuffer.clear();
    }
    
    // Update global objects
    for (auto& [typeId, objects] : m_objects)
    {
        for (auto& obj : objects)
            obj->onUpdate(deltaTime);
    }

    //// Update per‑window objects
    //for (auto& [windowId, typeMap] : m_windowObjects)
    //{
    //    for (auto& [typeId, objects] : typeMap)
    //    {
    //        for (auto& obj : objects)
    //            obj->onUpdate(deltaTime);
    //    }
    //}

    for (auto& comp : m_dirtyTransforms)
    {
        comp->updateWorldMatrix();
    }
    m_dirtyTransforms.clear();
}

dx3d::GameObject* dx3d::World::createGameObjectInternal(UniquePtr<GameObject>& object)
{
	if (!object) return {};

	auto ptr = object.get();
	auto typeId = ptr->getTypeId();

	// Store immediately in the main container
	m_objects[typeId].push_back(std::move(object));

	// Queue the onCreate event for later
    m_events.push_back({ ptr, EventType::Create });

	return ptr;
}

void dx3d::World::addComponentInternal(Component& component)
{
	auto typeId = component.getTypeId();
	m_components[typeId].push_back(&component);
}

void dx3d::World::addDirtyTransformInternal(TransformComponent& component)
{
	m_dirtyTransforms.push_back(&component);
}

dx3d::Component* const* dx3d::World::getComponentsInternal(size_t typeId, ui32* numComponents) const noexcept
{
	auto it = m_components.find(typeId);
	if (it != m_components.end())
	{
		*numComponents = static_cast<ui32>(it->second.size());
		return it->second.data();
	}

	*numComponents = 0u;
	return {};
}