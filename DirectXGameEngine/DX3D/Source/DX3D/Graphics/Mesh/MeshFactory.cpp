#include <DX3D/Graphics/Mesh/MeshFactory.h>
#include <cmath>

dx3d::RefPtr<dx3d::Mesh> dx3d::MeshFactory::createCubeMesh()
{
	std::vector<Vertex> vertices =
	{
		{{-0.5f,-0.5f,-0.5f}, {1,0,0,1}},
		{{-0.5f,0.5f,-0.5f}, {0,1,0,1} },
		{{0.5f,0.5f,-0.5f},  {0,0,1,1}},
		{{0.5f,-0.5f,-0.5f}, {1,0,1,1}},

		{{0.5f,-0.5f,0.5f}, {1,0,1,1}},
		{{0.5f,0.5f,0.5f}, {0,0,1,1}},
		{{-0.5f,0.5f,0.5f}, {0,1,0,1}},
		{{-0.5f,-0.5f,0.5f}, {1,0,0,1}}
	};

	std::vector<ui32> indices =
	{
		0,1,2,
		2,3,0,

		4,5,6,
		6,7,4,

		1,6,5,
		5,2,1,

		7,0,3,
		3,4,7,

		3,2,5,
		5,4,3,

		7,6,1,
		1,0,7
	};

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

			// Alternate colors for visual interest
			f32 colorR = (i % 2 == 0) ? 1.0f : 0.5f;
			f32 colorG = (j % 2 == 0) ? 1.0f : 0.5f;
			f32 colorB = ((i + j) % 2 == 0) ? 1.0f : 0.5f;

			vertices.push_back({ {x, y, z}, {colorR, colorG, colorB, 1.0f} });
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
	f32 colorValue = 0.7f;

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

			vertices.push_back({ {x, y, z_offset}, {colorValue, colorValue + 0.3f, colorValue, 1.0f} });
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

			vertices.push_back({ {x, y, z_offset}, {colorValue, colorValue, colorValue, 1.0f} });
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

			vertices.push_back({ {x, y, z_offset}, {colorValue + 0.2f, colorValue, colorValue, 1.0f} });
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

	const Vec4 cyclinderColor = {1.0f, 1.0f, 1.0f, 1.0f};

	f32 halfHeight = height / 2.0f;

	// Top circle
	vertices.push_back({ {0.0f, halfHeight, 0.0f}, cyclinderColor }); // center
	for (ui32 i = 0; i < segments; ++i)
	{
		f32 theta = TWO_PI * (i / static_cast<f32>(segments));
		f32 x = radius * cosf(theta);
		f32 z = radius * sinf(theta);
		vertices.push_back({ {x, halfHeight, z}, cyclinderColor });
	}

	// Bottom circle
	ui32 bottomCenterIdx = static_cast<ui32>(vertices.size());
	vertices.push_back({ {0.0f, -halfHeight, 0.0f}, cyclinderColor }); // center
	for (ui32 i = 0; i < segments; ++i)
	{
		f32 theta = TWO_PI * (i / static_cast<f32>(segments));
		f32 x = radius * cosf(theta);
		f32 z = radius * sinf(theta);
		vertices.push_back({ {x, -halfHeight, z}, cyclinderColor });
	}

	// Side vertices
	ui32 sideStartIdx = static_cast<ui32>(vertices.size());
	for (ui32 i = 0; i < segments; ++i)
	{
		f32 theta = TWO_PI * (i / static_cast<f32>(segments));
		f32 x = radius * cosf(theta);
		f32 z = radius * sinf(theta);

		vertices.push_back({ {x, halfHeight, z}, cyclinderColor });
		vertices.push_back({ {x, -halfHeight, z}, cyclinderColor });
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
		ui32 top2 = sideStartIdx + ((i + 1) % segments) * 2;
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

			// Checkerboard coloring
			f32 colorValue = ((x + y) % 2 == 0) ? 0.8f : 0.5f;
			vertices.push_back({ {posX, posY, posZ}, {colorValue, colorValue, colorValue, 1.0f} });
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
	vertices.push_back({ {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f} });

	// Circumference vertices
	for (ui32 i = 0; i <= segments; ++i)
	{
		f32 theta = TWO_PI * (i / static_cast<f32>(segments));
		f32 x = radius * cosf(theta);
		f32 z = radius * sinf(theta);

		// Alternating colors around the circle
		f32 colorValue = (i % 2 == 0) ? 0.9f : 0.6f;
		vertices.push_back({ {x, 0.0f, z}, {colorValue, colorValue, 1.0f, 1.0f} });
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
