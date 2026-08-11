#pragma once

#include <DX3D/Math/Vec3.h>

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace dx3d
{
	class GameObject;
	class World;
}

struct SceneObjectData
{
	size_t id{};
	size_t parentId{};
	std::string name;
	std::string type;
	std::string textureName;
	bool enabled{};
	bool physicsEnabled{};
	float pointLightIntensity{};
	float pointLightRange{};
	dx3d::Vec3 position{};
	dx3d::Vec3 rotation{};
	dx3d::Vec3 scale{};
};

class SceneSerializer
{
public:
	using ObjectTypeResolver = std::function<std::string(dx3d::GameObject&)>;

	static bool saveToJsonFile(
		const std::filesystem::path& path,
		dx3d::World& world,
		const ObjectTypeResolver& objectTypeResolver);

	static bool loadFromJsonFile(
		const std::filesystem::path& path,
		std::vector<SceneObjectData>& outObjects);
};
