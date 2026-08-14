#pragma once
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Identifiable.h>
#include <DX3D/Game/Component.h>

#include <unordered_map>

namespace dx3d
{
	class GameObject : public Identifiable
	{
		dx3d_typeid(GameObject)
	public:
		explicit GameObject(const GameObjectDesc& desc);

		template <typename T>
		T* createOrGetComponent() requires IsRegistered<Component, T>
		{
			auto c = getComponent<T>();
			if (c) return c;
			UniquePtr<Component> cp = std::make_unique<T>(ComponentDesc{
								{m_logger},
								*this,
								m_world,
								m_gameContext
				});
			return static_cast<T*>(createComponentInternal(cp));
		}

		template <typename T>
		T* getComponent() requires IsRegistered<Component, T>
		{
			return static_cast<T*>(getComponentInternal(T::GetTypeId()));
		}

		template <typename T>
		bool removeComponent() requires IsRegistered<Component, T>
		{
			return removeComponentInternal(T::GetTypeId());
		}

		TransformComponent& getTransform() noexcept;
		World& getWorld() noexcept;
		InputSystem& getInputSystem() noexcept;
		ResourceManager& getResourceManager() noexcept;

		uint32_t getWindowId() const noexcept { return m_windowId; }
		void setWindowId(uint32_t id) noexcept { m_windowId = id; }

		bool isEnabled() const noexcept { return m_enabled; }
		void setEnabled(bool enabled) noexcept { m_enabled = enabled; }

		bool isDeleted() const noexcept { return m_deleted; }
		void setDeleted(bool deleted) noexcept { m_deleted = deleted; }

		void setName(std::string name);
		std::string getName() { return m_name; }

		void setID(size_t _id) { m_ID = _id; }
		size_t getID() { return m_ID; }

		void setParent(GameObject* newParent);
		GameObject* getParent() const noexcept { return m_parent; }
		const std::vector<GameObject*>& getChildren() const noexcept { return m_children; }
		bool isDescendantOf(const GameObject* potentialAncestor) const;
		bool isActiveInHierarchy() const noexcept;

	protected:
		virtual void onCreate() {}
		virtual void onUpdate(f32 deltaTime) {}

	private:
		Component* createComponentInternal(UniquePtr<Component>& component);
		Component* getComponentInternal(size_t id);
		bool removeComponentInternal(size_t id);
	private:
		std::string m_name = "GameObject";
		size_t m_ID;
		InputSystem* m_windowInput;
		std::unordered_map<size_t, UniquePtr<Component>> m_components{};

		TransformComponent* m_transform{};
		
		GameContext m_gameContext;
		World& m_world;
		uint32_t m_windowId{ 0 };
		bool m_enabled{ true };
		bool m_deleted{ false };

		GameObject* m_parent{ nullptr };
		std::vector<GameObject*> m_children{};

		friend class World;
	};
}
