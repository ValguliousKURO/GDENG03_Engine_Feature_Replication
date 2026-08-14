#pragma once
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Identifiable.h>

namespace dx3d
{
	class Component : public Identifiable
	{
		dx3d_typeid(Component)
	public:
		explicit Component(const ComponentDesc& desc);
		virtual ~Component() = default;
		GameObject& getGameObject() noexcept;

		void setID(size_t _id) { m_ID = _id; }
		size_t getID() { return m_ID; }
	protected:
		size_t m_ID;
		GameObject& m_object;
		World& m_world;
		GameContext& m_context;
	};
}
