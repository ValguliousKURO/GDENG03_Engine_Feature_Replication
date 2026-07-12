#include <DX3D/Graphics/Mesh/MeshFactory.h>
#include <cmath>

dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createCubeMesh()
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

dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createSphereMesh(ui32 stacks, ui32 slices)
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




dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createCapsuleMesh(f32 radius, f32 height, ui32 segments, ui32 rings)
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

dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createPlaneMesh(f32 width, f32 height, ui32 widthSegments, ui32 heightSegments)
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

dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createCircleMesh(f32 radius, ui32 segments)
{
	std::vector<Vertex> vertices;
	std::vector<ui32> indices;

	constexpr f32 PI = 3.14159265359f;
	constexpr f32 TWO_PI = 2.0f * PI;

	// Center vertex
	vertices.push_back({ {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} });

	// Circumference vertices
	for (ui32 i = 0; i <= segments; ++i)
	{
		f32 theta = TWO_PI * (i / static_cast<f32>(segments));
		f32 x = radius * cosf(theta);
		f32 z = radius * sinf(theta);

		Vec2 uv(x / (2.0f * radius) + 0.5f, z / (2.0f * radius) + 0.5f);
		vertices.push_back({ {x, 0.0f, z}, uv, {0.0f, 1.0f, 0.0f} });
	}

	// Generate indices (fan triangulation from center)
	for (ui32 i = 1; i <= segments; ++i)
	{
		indices.push_back(0);
		indices.push_back(i);
		indices.push_back(i + 1);
	}

	return std::make_shared<Mesh>(vertices, indices);
}
