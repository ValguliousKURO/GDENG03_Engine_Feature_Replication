#include "SceneSerializer.h"

#include <DX3D/Component/CameraComponent.h>
#include <DX3D/Component/PhysicsComponent.h>
#include <DX3D/Component/TransformComponent.h>
#include <DX3D/Component/MeshComponent.h>
#include <DX3D/Component/PointLightComponent.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Game/World.h>
#include <DX3D/Resource/MaterialResource.h>
#include <DX3D/Resource/TextureManager.h>

#include <algorithm>
#include <fstream>
#include <optional>
#include <sstream>

namespace
{
	std::string escapeJsonString(const std::string& value)
	{
		std::string escaped;
		escaped.reserve(value.size());
		for (const auto c : value)
		{
			switch (c)
			{
			case '\\': escaped += "\\\\"; break;
			case '"': escaped += "\\\""; break;
			case '\n': escaped += "\\n"; break;
			case '\r': escaped += "\\r"; break;
			case '\t': escaped += "\\t"; break;
			default: escaped += c; break;
			}
		}
		return escaped;
	}

	std::string unescapeJsonString(const std::string& value)
	{
		std::string unescaped;
		unescaped.reserve(value.size());
		for (size_t i = 0; i < value.size(); ++i)
		{
			if (value[i] != '\\' || i + 1 >= value.size())
			{
				unescaped += value[i];
				continue;
			}

			const auto next = value[++i];
			switch (next)
			{
			case '\\': unescaped += '\\'; break;
			case '"': unescaped += '"'; break;
			case 'n': unescaped += '\n'; break;
			case 'r': unescaped += '\r'; break;
			case 't': unescaped += '\t'; break;
			default: unescaped += next; break;
			}
		}
		return unescaped;
	}

	std::optional<std::string> readJsonStringField(const std::string& line, const std::string& fieldName)
	{
		const auto key = "\"" + fieldName + "\"";
		auto keyPos = line.find(key);
		if (keyPos == std::string::npos) return std::nullopt;

		auto colonPos = line.find(':', keyPos + key.size());
		if (colonPos == std::string::npos) return std::nullopt;

		auto quoteStart = line.find('"', colonPos + 1);
		if (quoteStart == std::string::npos) return std::nullopt;

		std::string raw;
		bool escaped = false;
		for (size_t i = quoteStart + 1; i < line.size(); ++i)
		{
			const auto c = line[i];
			if (escaped)
			{
				raw += '\\';
				raw += c;
				escaped = false;
				continue;
			}
			if (c == '\\')
			{
				escaped = true;
				continue;
			}
			if (c == '"')
			{
				return unescapeJsonString(raw);
			}
			raw += c;
		}

		return std::nullopt;
	}

	std::optional<size_t> readJsonSizeTField(const std::string& line, const std::string& fieldName)
	{
		const auto key = "\"" + fieldName + "\"";
		auto keyPos = line.find(key);
		if (keyPos == std::string::npos) return std::nullopt;

		auto colonPos = line.find(':', keyPos + key.size());
		if (colonPos == std::string::npos) return std::nullopt;

		auto valueStart = line.find_first_not_of(" \t", colonPos + 1);
		if (valueStart == std::string::npos) return std::nullopt;

		try
		{
			return static_cast<size_t>(std::stoull(line.substr(valueStart)));
		}
		catch (const std::exception&)
		{
			return std::nullopt;
		}
	}

	std::optional<bool> readJsonBoolField(const std::string& line, const std::string& fieldName)
	{
		const auto key = "\"" + fieldName + "\"";
		auto keyPos = line.find(key);
		if (keyPos == std::string::npos) return std::nullopt;

		auto colonPos = line.find(':', keyPos + key.size());
		if (colonPos == std::string::npos) return std::nullopt;

		auto valueStart = line.find_first_not_of(" \t", colonPos + 1);
		if (valueStart == std::string::npos) return std::nullopt;

		if (line.compare(valueStart, 4, "true") == 0) return true;
		if (line.compare(valueStart, 5, "false") == 0) return false;
		return std::nullopt;
	}

	std::optional<float> readJsonFloatField(const std::string& line, const std::string& fieldName)
	{
		const auto key = "\"" + fieldName + "\"";
		auto keyPos = line.find(key);
		if (keyPos == std::string::npos) return std::nullopt;

		auto colonPos = line.find(':', keyPos + key.size());
		if (colonPos == std::string::npos) return std::nullopt;

		auto valueStart = line.find_first_not_of(" \t", colonPos + 1);
		if (valueStart == std::string::npos) return std::nullopt;

		try
		{
			return std::stof(line.substr(valueStart));
		}
		catch (const std::exception&)
		{
			return std::nullopt;
		}
	}

	std::optional<dx3d::Vec3> readJsonVec3Field(const std::string& line, const std::string& fieldName)
	{
		const auto key = "\"" + fieldName + "\"";
		auto keyPos = line.find(key);
		if (keyPos == std::string::npos) return std::nullopt;

		auto openBracket = line.find('[', keyPos + key.size());
		auto closeBracket = line.find(']', openBracket);
		if (openBracket == std::string::npos || closeBracket == std::string::npos) return std::nullopt;

		std::string values = line.substr(openBracket + 1, closeBracket - openBracket - 1);
		std::replace(values.begin(), values.end(), ',', ' ');

		std::istringstream stream(values);
		dx3d::Vec3 result{};
		if (!(stream >> result.x >> result.y >> result.z)) return std::nullopt;

		return result;
	}

	bool readSceneObjectFromJson(std::istream& file, SceneObjectData& data)
	{
		std::string line;
		while (std::getline(file, line))
		{
			if (line.find('{') != std::string::npos) break;
			if (line.find(']') != std::string::npos) return false;
		}

		if (!file) return false;

		while (std::getline(file, line))
		{
			if (line.find('}') != std::string::npos) return true;

			if (auto value = readJsonSizeTField(line, "id")) data.id = *value;
			else if (auto value = readJsonSizeTField(line, "parentId")) data.parentId = *value;
			else if (auto value = readJsonStringField(line, "name")) data.name = *value;
			else if (auto value = readJsonStringField(line, "type")) data.type = *value;
			else if (auto value = readJsonStringField(line, "texture")) data.textureName = *value;
			else if (auto value = readJsonBoolField(line, "enabled")) data.enabled = *value;
			else if (auto value = readJsonBoolField(line, "physicsEnabled")) data.physicsEnabled = *value;
			else if (auto value = readJsonFloatField(line, "pointLightIntensity")) data.pointLightIntensity = *value;
			else if (auto value = readJsonFloatField(line, "pointLightRange")) data.pointLightRange = *value;
			else if (auto value = readJsonVec3Field(line, "position")) data.position = *value;
			else if (auto value = readJsonVec3Field(line, "rotation")) data.rotation = *value;
			else if (auto value = readJsonVec3Field(line, "scale")) data.scale = *value;
		}

		return false;
	}
}

bool SceneSerializer::saveToJsonFile(
	const std::filesystem::path& path,
	dx3d::World& world,
	const ObjectTypeResolver& objectTypeResolver)
{
	std::error_code error;
	std::filesystem::create_directories(path.parent_path(), error);
	if (error) return false;

	std::ofstream file(path);
	if (!file) return false;

	file << "{\n";
	file << "  \"version\": 1,\n";
	file << "  \"objects\": [\n";

	bool wroteObject = false;

	for (const auto& [typeId, objects] : world.getGameObjectList())
	{
		for (const auto& objectPtr : objects)
		{
			auto* object = objectPtr.get();
			if (!object || object->isDeleted()) continue;
			if (object->getComponent<dx3d::CameraComponent>()) continue;

			const auto objectType = objectTypeResolver(*object);
			if (objectType.empty()) continue;

			auto& transform = object->getTransform();
			const auto position = transform.getPosition();
			const auto rotation = transform.getRotation();
			const auto scale = transform.getScale();
			auto* physics = object->getComponent<dx3d::PhysicsComponent>();
			auto* parent = object->getParent();
			const auto parentId = (parent && !parent->isDeleted() && !parent->getComponent<dx3d::CameraComponent>())
				? parent->getID()
				: 0;
			std::string textureName;
			if (auto* mesh = object->getComponent<dx3d::MeshComponent>())
			{
				if (auto* material = mesh->getMaterial())
				{
					if (auto* texture = material->getTexture(0))
					{
						textureName = dx3d::TextureManager::getInstance().getStringKey(texture);
						if (textureName == "NOT_FOUND")
						{
							textureName.clear();
						}
					}
				}
			}
			auto* pointLight = object->getComponent<dx3d::PointLightComponent>();

			if (wroteObject)
			{
				file << ",\n";
			}

			file << "    {\n"
				<< "      \"id\": " << object->getID() << ",\n"
				<< "      \"parentId\": " << parentId << ",\n"
				<< "      \"name\": \"" << escapeJsonString(object->getName()) << "\",\n"
				<< "      \"type\": \"" << escapeJsonString(objectType) << "\",\n"
				<< "      \"texture\": \"" << escapeJsonString(textureName) << "\",\n"
				<< "      \"enabled\": " << (object->isEnabled() ? "true" : "false") << ",\n"
				<< "      \"physicsEnabled\": " << ((physics && physics->isPhysicsEnabled()) ? "true" : "false") << ",\n"
				<< "      \"pointLightIntensity\": " << (pointLight ? pointLight->getIntensity() : 0.0f) << ",\n"
				<< "      \"pointLightRange\": " << (pointLight ? pointLight->getRange() : 0.0f) << ",\n"
				<< "      \"position\": [" << position.x << ", " << position.y << ", " << position.z << "],\n"
				<< "      \"rotation\": [" << rotation.x << ", " << rotation.y << ", " << rotation.z << "],\n"
				<< "      \"scale\": [" << scale.x << ", " << scale.y << ", " << scale.z << "]\n"
				<< "    }";

			wroteObject = true;
		}
	}

	file << "\n  ]\n";
	file << "}\n";

	return true;
}

bool SceneSerializer::loadFromJsonFile(
	const std::filesystem::path& path,
	std::vector<SceneObjectData>& outObjects)
{
	std::ifstream file(path);
	if (!file) return false;

	std::vector<SceneObjectData> loadedObjects;
	std::string line;
	while (std::getline(file, line))
	{
		if (line.find("\"objects\"") == std::string::npos) continue;

		while (true)
		{
			SceneObjectData data;
			const auto beforeRead = file.tellg();
			if (!readSceneObjectFromJson(file, data)) break;
			if (data.name.empty() || data.type.empty()) return false;
			loadedObjects.push_back(data);
			if (beforeRead == file.tellg()) break;
		}
		break;
	}

	outObjects = std::move(loadedObjects);
	return true;
}
