#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <DX3D/Core/Base.h>
#include <DX3D/Graphics/Mesh/Mesh.h>

namespace dx3d
{
	class MeshFactory final : public Base
	{
	public:
		explicit MeshFactory(const MeshFactoryDesc& desc);

		RefPtr<Mesh> createCubeMesh();
		RefPtr<Mesh> createSphereMesh(ui32 stacks = 20, ui32 slices = 20);
		RefPtr<Mesh> createCapsuleMesh(f32 radius = 0.5f, f32 height = 2.0f, ui32 segments = 16, ui32 rings = 8);
		RefPtr<Mesh> createCylinderMesh(f32 radius = 0.5f, f32 height = 2.0f, ui32 segments = 32);
		RefPtr<Mesh> createPlaneMesh(f32 width = 1.0f, f32 height = 1.0f, ui32 widthSegments = 1, ui32 heightSegments = 1);
		RefPtr<Mesh> createCircleMesh(f32 radius = 0.5f, ui32 segments = 32);

		// CUSTOM MESH LOADING HERE // testing

		//RefPtr<Mesh> loadMeshFromFile(const std::string& filepath);
		void loadAll();
		void loadAllObjMeshes(const std::string& directory);
		RefPtr<Mesh> getCustomMesh(const std::string& name) ;
		std::vector<std::string> getCustomMeshNames() const;




	private:
		// add a storage system like unordered maps or smth to store Obj files where engine can reference or load instead of setting filepath every time.
		bool loadMeshFromFile(const std::string& name, const std::string& filepath);
		RefPtr<Mesh> createMesh(const std::vector<Vertex>& vertices, const std::vector<ui32>& indices);

		// for storage of object meshes
	private:
		std::unordered_map<std::string, RefPtr<Mesh>> m_ObjMesh;// [game, obj data]

	
	};
}
