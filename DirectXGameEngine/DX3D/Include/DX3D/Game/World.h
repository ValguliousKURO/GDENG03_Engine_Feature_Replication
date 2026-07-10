#pragma once
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Identifiable.h>
#include <unordered_map>
#include <vector>


namespace dx3d
{
	class World final : public Base
	{
	public:
		explicit World(const WorldDesc& desc);

		template <typename T>
		T* createGameObject() requires IsRegistered<GameObject, T>
		{
			static_assert(std::is_base_of<GameObject, T>::value, "T must inherit from dx3d::GameObject.");
			static_assert(HasTypeId<T>, "T needs a unique TypeId. Make sure you added dx3d_typeid and applied it to the correct class.");
			UniquePtr<GameObject> e = std::make_unique<T>(GameObjectDesc{
				{m_logger},
				m_gameContext,
				*this
				});
			return static_cast<T*>(createGameObjectInternal(e));
		}

		// Game Object for a specific window
		template <typename T>
		T* createGameObjectForWindow(uint32_t windowId, InputSystem& inputSystem) requires IsRegistered<GameObject, T>
		{
			UniquePtr<GameObject> e = std::make_unique<T>(GameObjectDesc{
				{m_logger},
				m_gameContext,
				*this,
				&inputSystem // pass per‑window input
				});

			// Register components just like global objects
			auto* obj = static_cast<T*>(createGameObjectInternal(e));

			// Store in per‑window bucket
			auto typeId = T::GetTypeId();
			m_windowObjects[windowId][typeId].push_back(std::move(e));

			return obj;
		}


		template <typename T> requires IsRegistered<Component, T>
		T* const* getComponents(ui32& numComponents) const noexcept
		{
			return reinterpret_cast<T* const*>(getComponentsInternal(T::GetTypeId(), &numComponents));
		}

		void update(f32 deltaTime);
	private:
		GameObject* createGameObjectInternal(UniquePtr<GameObject>& object);
		void addComponentInternal(Component& component);
		void addDirtyTransformInternal(TransformComponent& component);

		Component* const* getComponentsInternal(size_t typeId, ui32* numComponents) const noexcept;
	private:
		enum class EventType
		{
			Create = 0
		};
		struct GameObjectEvent
		{
			GameObject* object{};
			// size_t pendingObjectIndex{};
			EventType eventType{};
		};

	private:
		GameContext m_gameContext;
		// Shared Game objects
		std::unordered_map<size_t, std::vector<UniquePtr<GameObject>>> m_objects{};

		// Game objects bound to a specific window ID
		std::unordered_map<uint32_t, std::unordered_map<size_t, std::vector<UniquePtr<GameObject>>>> m_windowObjects{};

		std::unordered_map<size_t, std::vector<Component*>> m_components{};

		std::vector<TransformComponent*> m_dirtyTransforms{};

		std::vector<UniquePtr<GameObject>> m_pendingObjects;
		std::vector<UniquePtr<GameObject>> m_pendingObjectsSwapBuffer;

		std::vector<GameObjectEvent> m_events{};
		std::vector<GameObjectEvent> m_eventsSwapBuffer{};

		friend class GameObject;
		friend class TransformComponent;
	};
}
