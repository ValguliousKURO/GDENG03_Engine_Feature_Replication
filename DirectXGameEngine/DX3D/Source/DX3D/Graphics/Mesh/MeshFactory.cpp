#include <DX3D/Graphics/Mesh/MeshFactory.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace
{
	struct ObjIndex { int position = 0; int texcoord = 0; int normal = 0; };
	int resolveObjIndex(int index, size_t count);
	bool parseObjIndex(const std::string& token, ObjIndex& result);
	dx3d::Vec2 generateFallbackTexcoord(const dx3d::Vec3& position, const dx3d::Vec3& minimum, const dx3d::Vec3& maximum);
}

dx3d::MeshFactory::MeshFactory(const MeshFactoryDesc& desc) : Base(desc.base)
{
}

// set all loading obj here


void dx3d::MeshFactory::loadAll()
{
	this->loadAllObjMeshes("Assets/ObjFiles");
}

void dx3d::MeshFactory::loadAllObjMeshes(const std::string& directory)
{
	namespace fs = std::filesystem;

	std::error_code error;
	if (!fs::exists(directory, error) || !fs::is_directory(directory, error))
	{
		return;
	}

	std::vector<fs::path> objFiles;
	for (const auto& entry : fs::directory_iterator(directory, error))
	{
		if (error) return;
		if (!entry.is_regular_file(error)) continue;

		auto path = entry.path();
		if (path.extension() == ".obj" || path.extension() == ".OBJ")
		{
			objFiles.push_back(path);
		}
	}

	std::sort(objFiles.begin(), objFiles.end());
	for (const auto& path : objFiles)
	{
		const auto name = path.stem().string();
		loadMeshFromFile(name, path.string());
	}
}

dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createCubeMesh() //Generate a cube mesh
{
	std::vector<Vertex> vertices;
	std::vector<ui32> indices;

	// Front Face (normal: 0, 0, -1)
	vertices.push_back({ {-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f} });
	vertices.push_back({ {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} });
	vertices.push_back({ { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f} });
	vertices.push_back({ { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} });

	// Back Face (normal: 0, 0, 1)
	vertices.push_back({ { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} });
	vertices.push_back({ { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} });
	vertices.push_back({ {-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} });
	vertices.push_back({ {-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} });

	// Top Face (normal: 0, 1, 0)
	vertices.push_back({ {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f} });
	vertices.push_back({ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} });
	vertices.push_back({ { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} });
	vertices.push_back({ { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} });

	// Bottom Face (normal: 0, -1, 0)
	vertices.push_back({ {-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f} });
	vertices.push_back({ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f} });
	vertices.push_back({ { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f} });
	vertices.push_back({ { 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} });

	// Left Face (normal: -1, 0, 0)
	vertices.push_back({ {-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} });
	vertices.push_back({ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} });
	vertices.push_back({ {-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} });
	vertices.push_back({ {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} });

	// Right Face (normal: 1, 0, 0)
	vertices.push_back({ { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} });
	vertices.push_back({ { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });
	vertices.push_back({ { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });
	vertices.push_back({ { 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} });

	for (ui32 f = 0; f < 6; ++f)
	{
		ui32 base = f * 4;
		indices.push_back(base + 0);
		indices.push_back(base + 1);
		indices.push_back(base + 2);

		indices.push_back(base + 2);
		indices.push_back(base + 3);
		indices.push_back(base + 0);
	}

	return std::make_shared<Mesh>(vertices, indices);
}

dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createSphereMesh(ui32 stacks, ui32 slices) //Generate a sphere mesh
{
	std::vector<Vertex> vertices;
	std::vector<ui32> indices;

	constexpr f32 PI = 3.14159265359f;
	constexpr f32 TWO_PI = 2.0f * PI;

	// Generate vertices
	for (ui32 i = 0; i <= stacks; ++i)
	{
		f32 stackAngle = PI / 2.0f - (i * PI / stacks);
		f32 xy = 0.5f * cosf(stackAngle);
		f32 z = 0.5f * sinf(stackAngle);

		for (ui32 j = 0; j <= slices; ++j)
		{
			f32 sliceAngle = (j * TWO_PI / slices);
			f32 x = xy * cosf(sliceAngle);
			f32 y = xy * sinf(sliceAngle);

			Vec3 pos(x, y, z);
			Vec3 normal = Vec3::normalize(pos);
			Vec2 texcoord((f32)j / slices, (f32)i / stacks);

			vertices.push_back({ pos, texcoord, normal });
		}
	}

	// Generate indices
	for (ui32 i = 0; i < stacks; ++i)
	{
		ui32 k1 = i * (slices + 1);
		ui32 k2 = k1 + slices + 1;

		for (ui32 j = 0; j < slices; ++j)
		{
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			if (i != (stacks - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}

			k1++;
			k2++;
		}
	}

	return std::make_shared<Mesh>(vertices, indices);
}




dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createCapsuleMesh(f32 radius, f32 height, ui32 segments, ui32 rings) //Generate a capsule mesh
{
	std::vector<Vertex> vertices;
	std::vector<ui32> indices;

	constexpr f32 PI = 3.14159265359f;
	constexpr f32 TWO_PI = 2.0f * PI;

	f32 halfHeight = height / 2.0f;

	// Top hemisphere
	for (ui32 i = 0; i <= rings; ++i)
	{
		f32 phi = (PI / 2.0f) * (i / static_cast<f32>(rings));
		f32 z_offset = halfHeight + radius * sinf(phi);

		for (ui32 j = 0; j <= segments; ++j)
		{
			f32 theta = TWO_PI * (j / static_cast<f32>(segments));
			f32 r = radius * cosf(phi);
			f32 x = r * cosf(theta);
			f32 y = r * sinf(theta);

			Vec3 pos(x, y, z_offset);
			Vec3 normal = Vec3::normalize(Vec3(x, y, radius * sinf(phi)));
			Vec2 uv((f32)j / segments, (z_offset + halfHeight + radius) / (height + 2.0f * radius));
			vertices.push_back({ pos, uv, normal });
		}
	}

	// Cylinder body
	for (ui32 i = 0; i <= 1; ++i) // two rings: top and bottom of cylinder
	{
		f32 z_offset = (i == 0 ? halfHeight : -halfHeight);

		for (ui32 j = 0; j <= segments; ++j)
		{
			f32 theta = TWO_PI * (j / static_cast<f32>(segments));
			f32 x = radius * cosf(theta);
			f32 y = radius * sinf(theta);

			Vec3 pos(x, y, z_offset);
			Vec3 normal = Vec3::normalize(Vec3(x, y, 0.0f));
			Vec2 uv((f32)j / segments, (z_offset + halfHeight + radius) / (height + 2.0f * radius));
			vertices.push_back({ pos, uv, normal });
		}
	}

	// Bottom hemisphere
	for (ui32 i = 1; i <= rings; ++i)
	{
		f32 phi = -(PI / 2.0f) * (i / static_cast<f32>(rings));
		f32 z_offset = -halfHeight + radius * sinf(phi);

		for (ui32 j = 0; j <= segments; ++j)
		{
			f32 theta = TWO_PI * (j / static_cast<f32>(segments));
			f32 r = radius * cosf(phi);
			f32 x = r * cosf(theta);
			f32 y = r * sinf(theta);

			Vec3 pos(x, y, z_offset);
			Vec3 normal = Vec3::normalize(Vec3(x, y, radius * sinf(phi)));
			Vec2 uv((f32)j / segments, (z_offset + halfHeight + radius) / (height + 2.0f * radius));
			vertices.push_back({ pos, uv, normal });
		}
	}

	// Indices for cylinder
	ui32 cylStart = (rings + 1) * (segments + 1);
	for (ui32 j = 0; j < segments; ++j)
	{
		ui32 k1 = cylStart + j;
		ui32 k2 = k1 + (segments + 1);

		indices.push_back(k1);
		indices.push_back(k2);
		indices.push_back(k1 + 1);

		indices.push_back(k1 + 1);
		indices.push_back(k2);
		indices.push_back(k2 + 1);
	}
	
	// Indices for top hemisphere (reversed winding)
	for (ui32 i = 0; i < rings; ++i)
	{
		ui32 k1 = i * (segments + 1);
		ui32 k2 = k1 + segments + 1;

		for (ui32 j = 0; j < segments; ++j)
		{
			indices.push_back(k1);
			indices.push_back(k1 + 1);
			indices.push_back(k2);

			indices.push_back(k1 + 1);
			indices.push_back(k2 + 1);
			indices.push_back(k2);

			k1++;
			k2++;
		}
	}

	// Indices for bottom hemisphere
	ui32 bottomStart = cylStart + 2 * (segments + 1);
	for (ui32 i = 0; i < rings; ++i)
	{
		ui32 k1 = bottomStart + i * (segments + 1);
		ui32 k2 = k1 + segments + 1;

		for (ui32 j = 0; j < segments; ++j)
		{
			indices.push_back(k1);
			indices.push_back(k2);
			indices.push_back(k1 + 1);

			indices.push_back(k1 + 1);
			indices.push_back(k2);
			indices.push_back(k2 + 1);

			k1++;
			k2++;
		}
	}

	return std::make_shared<Mesh>(vertices, indices);
}

dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createCylinderMesh(f32 radius, f32 height, ui32 segments)
{
	std::vector<Vertex> vertices;
	std::vector<ui32> indices;

	constexpr f32 PI = 3.14159265359f;
	constexpr f32 TWO_PI = 2.0f * PI;

	f32 halfHeight = height / 2.0f;

	// Top circle
	vertices.push_back({ {0.0f, halfHeight, 0.0f}, {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} }); // center
	for (ui32 i = 0; i < segments; ++i)
	{
		f32 theta = TWO_PI * (i / static_cast<f32>(segments));
		f32 x = radius * cosf(theta);
		f32 z = radius * sinf(theta);
		Vec2 uv(x / (2.0f * radius) + 0.5f, z / (2.0f * radius) + 0.5f);
		vertices.push_back({ {x, halfHeight, z}, uv, {0.0f, 1.0f, 0.0f} });
	}

	// Bottom circle
	ui32 bottomCenterIdx = static_cast<ui32>(vertices.size());
	vertices.push_back({ {0.0f, -halfHeight, 0.0f}, {0.5f, 0.5f}, {0.0f, -1.0f, 0.0f} }); // center
	for (ui32 i = 0; i < segments; ++i)
	{
		f32 theta = TWO_PI * (i / static_cast<f32>(segments));
		f32 x = radius * cosf(theta);
		f32 z = radius * sinf(theta);
		Vec2 uv(x / (2.0f * radius) + 0.5f, z / (2.0f * radius) + 0.5f);
		vertices.push_back({ {x, -halfHeight, z}, uv, {0.0f, -1.0f, 0.0f} });
	}

	// Side vertices
	ui32 sideStartIdx = static_cast<ui32>(vertices.size());
	for (ui32 i = 0; i <= segments; ++i) // wrap around by using <= segments so we get clean UV wrapping on side
	{
		f32 theta = TWO_PI * (i / static_cast<f32>(segments));
		f32 x = radius * cosf(theta);
		f32 z = radius * sinf(theta);

		Vec3 normal = Vec3::normalize(Vec3(x, 0.0f, z));
		Vec2 uv1((f32)i / segments, 0.0f);
		Vec2 uv2((f32)i / segments, 1.0f);

		vertices.push_back({ {x, halfHeight, z}, uv1, normal });
		vertices.push_back({ {x, -halfHeight, z}, uv2, normal });
	}

	// Top cap (outward)
	for (ui32 i = 1; i < segments; ++i)
	{
		indices.push_back(0);
		indices.push_back(i);
		indices.push_back(i + 1);
	}
	indices.push_back(0);
	indices.push_back(segments);
	indices.push_back(1);

	// Top cap (inward, reversed winding)
	for (ui32 i = 1; i < segments; ++i)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(segments);

	// Bottom cap (outward)
	for (ui32 i = 1; i < segments; ++i)
	{
		indices.push_back(bottomCenterIdx);
		indices.push_back(bottomCenterIdx + i);
		indices.push_back(bottomCenterIdx + i + 1);
	}
	indices.push_back(bottomCenterIdx);
	indices.push_back(bottomCenterIdx + segments);
	indices.push_back(bottomCenterIdx + 1);

	// Bottom cap (inward, reversed winding)
	for (ui32 i = 1; i < segments; ++i)
	{
		indices.push_back(bottomCenterIdx);
		indices.push_back(bottomCenterIdx + i + 1);
		indices.push_back(bottomCenterIdx + i);
	}
	indices.push_back(bottomCenterIdx);
	indices.push_back(bottomCenterIdx + 1);
	indices.push_back(bottomCenterIdx + segments);

	// Side faces
	for (ui32 i = 0; i < segments; ++i)
	{
		ui32 top1 = sideStartIdx + i * 2;
		ui32 bottom1 = top1 + 1;
		ui32 top2 = sideStartIdx + (i + 1) * 2;
		ui32 bottom2 = top2 + 1;

		indices.push_back(top1);
		indices.push_back(bottom1);
		indices.push_back(top2);

		indices.push_back(top2);
		indices.push_back(bottom1);
		indices.push_back(bottom2);
	}

	return std::make_shared<Mesh>(vertices, indices);
}

dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createPlaneMesh(f32 width, f32 height, ui32 widthSegments, ui32 heightSegments) //Generate a flat plane mesh
{
	std::vector<Vertex> vertices;
	std::vector<ui32> indices;

	f32 halfWidth = width / 2.0f;
	f32 halfHeight = height / 2.0f;
	f32 widthStep = width / widthSegments;
	f32 heightStep = height / heightSegments;

	// Generate vertices
	for (ui32 y = 0; y <= heightSegments; ++y)
	{
		for (ui32 x = 0; x <= widthSegments; ++x)
		{
			f32 posX = -halfWidth + (x * widthStep);
			f32 posY = 0.0f;
			f32 posZ = -halfHeight + (y * heightStep);

			Vec2 uv((f32)x / widthSegments, (f32)y / heightSegments);
			vertices.push_back({ {posX, posY, posZ}, uv, {0.0f, 1.0f, 0.0f} });
		}
	}

	// Generate indices
	for (ui32 y = 0; y < heightSegments; ++y)
	{
		for (ui32 x = 0; x < widthSegments; ++x)
		{
			ui32 a = y * (widthSegments + 1) + x;
			ui32 b = a + 1;
			ui32 c = a + (widthSegments + 1);
			ui32 d = c + 1;

			indices.push_back(a);
			indices.push_back(c);
			indices.push_back(b);

			indices.push_back(b);
			indices.push_back(c);
			indices.push_back(d);
		}
	}

	return std::make_shared<Mesh>(vertices, indices);
}

dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createCircleMesh(f32 radius, ui32 segments) //Generate a flat circle mesh
{
	std::vector<Vertex> vertices;
	std::vector<ui32> indices;

	constexpr f32 PI = 3.14159265359f;
	constexpr f32 TWO_PI = 2.0f * PI;

	//Center vertex
	vertices.push_back({ {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} });

	//Circumference vertices
	for (ui32 i = 0; i <= segments; ++i)
	{
		f32 theta = TWO_PI * (i / static_cast<f32>(segments));
		f32 x = radius * cosf(theta);
		f32 z = radius * sinf(theta);

		Vec2 uv(x / (2.0f * radius) + 0.5f, z / (2.0f * radius) + 0.5f);
		vertices.push_back({ {x, 0.0f, z}, uv, {0.0f, 1.0f, 0.0f} });
	}

	//Generate indices (fan triangulation from center)
	for (ui32 i = 1; i <= segments; ++i)
	{
		indices.push_back(0);
		indices.push_back(i);
		indices.push_back(i + 1);
	}

	return std::make_shared<Mesh>(vertices, indices);
}

//Get the mesh here
dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::getCustomMesh(const std::string& name) 
{
	 auto mesh = m_ObjMesh.find(name);
	return mesh != m_ObjMesh.end() ? mesh->second : nullptr; //if entry in obj is found and the stored value is not empty
}

std::vector<std::string> dx3d::MeshFactory::getCustomMeshNames() const
{
	std::vector<std::string> names;
	names.reserve(m_ObjMesh.size());
	for (const auto& [name, mesh] : m_ObjMesh)
	{
		if (mesh) names.push_back(name);
	}
	std::sort(names.begin(), names.end());
	return names;
}


//custom OBJ

bool dx3d::MeshFactory::loadMeshFromFile(const std::string& name, const std::string& filepath)
{
	std::ifstream file(filepath);
	if (!file) return false;

	std::vector<Vec3> positions;
	std::vector<Vec2> texcoords;
	std::vector<Vec3> normals;
	std::vector<Vertex> vertices;
	std::vector<ui32> indices;
	std::unordered_map<std::string, ui32> vertexCache;
	// position bounds
	Vec3 positionMinimum{};
	Vec3 positionMaximum{};
	bool hasPositionBounds = false;
	std::string line;

	while (std::getline(file, line)) // while file exists
	{
		std::istringstream stream(line); // read and parse data
		std::string command;
		stream >> command;
		if (command == "v") // object position bounds of an obj
		{
			Vec3 position;
			if (!(stream >> position.x >> position.y >> position.z)) return false;
			positions.push_back(position);
			if (!hasPositionBounds)
			{
				positionMinimum = positionMaximum = position;
				hasPositionBounds = true;
			}
			else
			{
				positionMinimum.x = std::min(positionMinimum.x, position.x);
				positionMinimum.y = std::min(positionMinimum.y, position.y);
				positionMinimum.z = std::min(positionMinimum.z, position.z);
				positionMaximum.x = std::max(positionMaximum.x, position.x);
				positionMaximum.y = std::max(positionMaximum.y, position.y);
				positionMaximum.z = std::max(positionMaximum.z, position.z);
			}
		}
		else if (command == "vt") // texture coordinates
		{
			Vec2 texcoord;
			if (!(stream >> texcoord.x >> texcoord.y)) return false;
			// OBJ UVs start at the lower edge; Direct3D texture coordinates start at the top.
			texcoord.y = 1.0f - texcoord.y; // from top to bottom
			texcoords.push_back(texcoord);
		}
		else if (command == "vn") // normals
		{
			Vec3 normal;
			if (!(stream >> normal.x >> normal.y >> normal.z)) return false;
			normals.push_back(Vec3::normalize(normal));
		}
		
		
		else if (command == "f") // faces
		{
			std::vector<ObjIndex> face;
			std::string token;
			while (stream >> token)
			{
				// OBJ permits an inline comment after a face definition.
				if (token.starts_with('#')) break;
				ObjIndex index;
				if (!parseObjIndex(token, index)) return false;
				index.position = resolveObjIndex(index.position, positions.size());
				index.texcoord = index.texcoord ? resolveObjIndex(index.texcoord, texcoords.size()) : -1;
				index.normal = index.normal ? resolveObjIndex(index.normal, normals.size()) : -1;
				if (index.position < 0 || index.position >= static_cast<int>(positions.size()) || index.texcoord >= static_cast<int>(texcoords.size()) || index.normal >= static_cast<int>(normals.size())) return false;
				face.push_back(index);
			}
			if (face.size() < 3) return false;
			// Fan triangulation supports triangles, quads, and convex OBJ polygons.
			for (size_t i = 1; i + 1 < face.size(); ++i)
			{
				const ObjIndex triangle[] = { face[0], face[i], face[i + 1] };
				const Vec3& a = positions[triangle[0].position]; const Vec3& b = positions[triangle[1].position]; const Vec3& c = positions[triangle[2].position];
				const Vec3 generatedNormal = Vec3::normalize(Vec3((b.y - a.y) * (c.z - a.z) - (b.z - a.z) * (c.y - a.y), (b.z - a.z) * (c.x - a.x) - (b.x - a.x) * (c.z - a.z), (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)));
				for (const ObjIndex& index : triangle)
				{
					// Do not share vertices without OBJ normals: each face needs its own flat normal.
					const std::string key = index.normal >= 0 ? std::to_string(index.position) + "/" + std::to_string(index.texcoord) + "/" + std::to_string(index.normal) : std::string{};
					auto cached = key.empty() ? vertexCache.end() : vertexCache.find(key);
					if (cached != vertexCache.end()) { indices.push_back(cached->second); continue; }
					const Vec3& position = positions[index.position];
					const Vec2 texcoord = index.texcoord >= 0
						? texcoords[index.texcoord]
						: generateFallbackTexcoord(position, positionMinimum, positionMaximum);
					const Vertex vertex{ position, texcoord, index.normal >= 0 ? normals[index.normal] : generatedNormal };
					const ui32 vertexIndex = static_cast<ui32>(vertices.size());
					vertices.push_back(vertex); indices.push_back(vertexIndex);
					if (!key.empty()) vertexCache.emplace(key, vertexIndex);
				}
			}
		}
	}
	auto mesh = createMesh(vertices, indices);
	if (!mesh) return false;

	m_ObjMesh[name] = mesh;
	return true;
}

dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createMesh(const std::vector<Vertex>& vertices, const std::vector<ui32>& indices)
{
	if (vertices.empty() || indices.empty() || indices.size() % 3 != 0)
		return nullptr;

	for (const auto index : indices)
	{
		if (index >= vertices.size())
			return nullptr;
	}

	return std::make_shared<Mesh>(vertices, indices);
}


///HELPER FUNCTIONS FOR LoadMesh

namespace
{
	// OBJ indices are one-based; negative values are relative to the latest record.
	int resolveObjIndex(int index, size_t count)
	{
		if (index > 0) return index - 1;
		if (index < 0) return static_cast<int>(count) + index;
		return -1;
	}

	bool parseObjIndex(const std::string& token, ObjIndex& result)
	{
		try
		{
			const auto firstSlash = token.find('/');
			if (firstSlash == std::string::npos) { result.position = std::stoi(token); return true; }
			result.position = std::stoi(token.substr(0, firstSlash));
			const auto secondSlash = token.find('/', firstSlash + 1);
			if (secondSlash == std::string::npos)
			{
				const auto uv = token.substr(firstSlash + 1);
				if (!uv.empty()) result.texcoord = std::stoi(uv);
				return true;
			}
			const auto uv = token.substr(firstSlash + 1, secondSlash - firstSlash - 1);
			const auto normal = token.substr(secondSlash + 1);
			if (!uv.empty()) result.texcoord = std::stoi(uv);
			if (!normal.empty()) result.normal = std::stoi(normal);
			return true;
		}
		catch (const std::exception&) { return false; }
	}

	// Some OBJ files, including the Stanford bunny, contain geometry only.  Give
	// those meshes a stable spherical projection instead of sampling texture
	// coordinate (0, 0) for every vertex.
	dx3d::Vec2 generateFallbackTexcoord(const dx3d::Vec3& position, const dx3d::Vec3& minimum, const dx3d::Vec3& maximum)
	{
		const dx3d::Vec3 centre{
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f
		};
		const dx3d::Vec3 halfExtent{
			std::max((maximum.x - minimum.x) * 0.5f, 0.0001f),
			std::max((maximum.y - minimum.y) * 0.5f, 0.0001f),
			std::max((maximum.z - minimum.z) * 0.5f, 0.0001f)
		};
		const dx3d::Vec3 direction = dx3d::Vec3::normalize({
			(position.x - centre.x) / halfExtent.x,
			(position.y - centre.y) / halfExtent.y,
			(position.z - centre.z) / halfExtent.z
			});

		constexpr dx3d::f32 Pi = 3.14159265358979323846f;
		return {
			std::atan2(direction.z, direction.x) / (2.0f * Pi) + 0.5f,
			0.5f - std::asin(std::clamp(direction.y, -1.0f, 1.0f)) / Pi
		};
	}
}
