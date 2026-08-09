#include "MainGame.h"
#include "Objects/Player.h"
#include "Objects/Camera.h"
#include <DX3D/Graphics/Mesh/MeshFactory.h>
#include <DX3D/Component/MeshComponent.h>
#include <DX3D/Component/CameraComponent.h>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <optional>
#include <algorithm>
#include <unordered_map>

#include <DX3D/UI/DebugWindowUI.h>
#include <DX3D/UI/HierarchyUI.h>
#include <DX3D/UI/MainMenuBarUI.h>
#include <DX3D/UI/SceneStateUI.h>

#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>
#include <DX3D/EventBroadcasting/Parameters.h>

#include <DX3D/Resource/TextureManager.h>

#include <reactphysics3d/reactphysics3d.h>
#include <DX3D/Component/PhysicsComponent.h>

namespace
{
	struct SerializableSceneObjectData
	{
		size_t id{};
		size_t parentId{};
		std::string name;
		std::string type;
		bool enabled{};
		bool physicsEnabled{};
		dx3d::Vec3 position{};
		dx3d::Vec3 rotation{};
		dx3d::Vec3 scale{};
	};

	std::filesystem::path findObjFilesDirectory()
	{
		namespace fs = std::filesystem;

		const auto current = fs::current_path();
		const std::vector<fs::path> candidates =
		{
			current / "Assets" / "ObjFiles",
			current.parent_path() / "Assets" / "ObjFiles",
			current.parent_path().parent_path() / "Assets" / "ObjFiles",
			current / "DirectXGameEngine" / "Assets" / "ObjFiles",
			current.parent_path() / "DirectXGameEngine" / "Assets" / "ObjFiles",
			current.parent_path().parent_path() / "DirectXGameEngine" / "Assets" / "ObjFiles"
		};

		for (const auto& candidate : candidates)
		{
			std::error_code error;
			if (fs::exists(candidate, error) && fs::is_directory(candidate, error))
			{
				return candidate;
			}
		}

		return current / "Assets" / "ObjFiles";
	}

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

	bool readSceneObjectFromJson(std::istream& file, SerializableSceneObjectData& data)
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
			else if (auto value = readJsonBoolField(line, "enabled")) data.enabled = *value;
			else if (auto value = readJsonBoolField(line, "physicsEnabled")) data.physicsEnabled = *value;
			else if (auto value = readJsonVec3Field(line, "position")) data.position = *value;
			else if (auto value = readJsonVec3Field(line, "rotation")) data.rotation = *value;
			else if (auto value = readJsonVec3Field(line, "scale")) data.scale = *value;
		}

		return false;
	}
}

MainGame::MainGame(const dx3d::GameDesc& desc) : dx3d::Game(desc)
{
}

void MainGame::onNewDisplay(dx3d::Display& display)
{
	auto& world = getWorld();

	auto camera = world.createGameObjectForWindow<Camera>(display.getID(), display.getInputSystem());
	camera->setName("Camera");
	auto* camComp = camera->createOrGetComponent<dx3d::CameraComponent>();
	display.setCamera(camComp);

	camera->getTransform().setPosition({ 0.0f, 1.0f, -2.0f });
	camera->getTransform().setRotation({ 0.0f, 0.0f, 0.0f });
	camComp->setProjectionMode(dx3d::ProjectionMode::Perspective);
	display.setRenderMode(dx3d::Display::RenderMode::Lit);

	display.getInputSystem().setCursorLocked(false);
	display.getInputSystem().setCursorVisible(true);

	//m_InspectorUIs[display.getID()] = std::make_unique<dx3d::InspectorUI>(dx3d::BaseDesc{ getLogger() });


}

void MainGame::onNewWorldView(std::string name)
{
	auto& world = getWorld();

	auto camera = world.createGameObject<Camera>();
	camera->setName(name + " Camera");
	auto* camComp = camera->createOrGetComponent<dx3d::CameraComponent>();

	camera->getTransform().setPosition({ 0.0f, 1.0f, -2.0f });
	camera->getTransform().setRotation({ 0.0f, 0.0f, 0.0f });
	camComp->setProjectionMode(dx3d::ProjectionMode::Perspective);

	addWorldView(name, camera->getID());
}

void MainGame::onCreate()
{
	Game::onCreate();
	auto& world = getWorld();
	std::filesystem::path base = std::filesystem::current_path().parent_path();

	dx3d::TextureManager::getInstance().loadAllTextures(getResourceManager());

	auto woodTex = dx3d::TextureManager::getInstance().getTexture("wood");
	auto floorTex = dx3d::TextureManager::getInstance().getTexture("floor");

	// UI initialize
	const auto objFilesDirectory = findObjFilesDirectory();
	getMeshFactory().loadAllObjMeshes(objFilesDirectory.string());
	m_availableObjModels = getMeshFactory().getCustomMeshNames();

	std::unique_ptr<dx3d::HierarchyUI> hierarchy_UI = std::make_unique<dx3d::HierarchyUI>(dx3d::BaseDesc{ getLogger() });
	hierarchy_UI->setGameObjectList(&world.getGameObjectList());
	hierarchy_UI->setObjModelNames(&m_availableObjModels);
	m_UIs.push_back(std::move(hierarchy_UI));
	m_UIs.push_back(std::make_unique<dx3d::DebugWindowUI>(dx3d::BaseDesc{ getLogger() }));

	m_UIs.push_back(std::make_unique<dx3d::MainMenuBarUI>(dx3d::BaseDesc{ getLogger() }));
	m_UIs.push_back(std::make_unique<dx3d::InspectorUI>(dx3d::BaseDesc{ getLogger() }));
	m_UIs.push_back(std::make_unique<dx3d::SceneStateUI>(dx3d::BaseDesc{ getLogger() }));

	// Create mesh resources (reusable)
	auto cubeMesh = getMeshFactory().createCubeMesh();
	m_spawnCubeMesh = cubeMesh;
	auto sphereMesh = getMeshFactory().createSphereMesh(20, 20);
	m_spawnSphereMesh = sphereMesh;
	auto capsuleMesh = getMeshFactory().createCapsuleMesh(0.5f, 2.0f);
	m_spawnCapsuleMesh = capsuleMesh;
	auto cylinderMesh = getMeshFactory().createCylinderMesh(0.5f, 2.0f);
	m_spawnCylinderMesh = cylinderMesh;
	auto planeMesh = getMeshFactory().createPlaneMesh(10.0f, 10.0f);
	m_spawnPlaneMesh = planeMesh;
	auto circleMesh = getMeshFactory().createCircleMesh(0.5f, 32);
	



	m_spawnMaterial = getResourceManager().createResourceFromFile<dx3d::MaterialResource>((base / "DirectXGameEngine/Game/Assets/Shaders/Basic.hlsl").c_str());
	if (m_spawnMaterial)
	{
		auto matData = dx3d::Vec3(1, 1, 1);
		m_spawnMaterial->setData(std::as_bytes(std::span{ &matData, 1 }));
		m_spawnMaterial->setTexture(0, woodTex);
	}

	{
		auto basicMat = getResourceManager().createResourceFromFile<dx3d::MaterialResource>((base/"DirectXGameEngine/Game/Assets/Shaders/Basic.hlsl").c_str());
		if (basicMat)
		{
			auto matData = dx3d::Vec3(1, 1, 1);
			basicMat->setData(std::as_bytes(std::span{ &matData, 1 }));
			basicMat->setTexture(0, floorTex);
		}

		auto floor = world.createGameObject<dx3d::GameObject>();
		auto floorMeshComp = floor->createOrGetComponent<dx3d::MeshComponent>();
		floorMeshComp->setMesh(planeMesh);
		floorMeshComp->setMaterial(basicMat);
		floor->getTransform().setScale({ 7.0f, 7.0f, 7.0f });
		floor->getTransform().setPosition({ 0, 0, 0 });

		///  test object

		//commented out since I don't have the model for me -Ira uncomment if you have the model in your project folder
		//auto armaDObject = world.createGameObject<dx3d::GameObject>();
		//
		//armaDObject->createOrGetComponent<dx3d::MeshComponent>()->setMesh(getMeshFactory().getCustomMesh("Armadillo"));
		//armaDObject->createOrGetComponent<dx3d::MeshComponent>()->setMaterial(basicMat);
		//armaDObject->getTransform().setScale({ 0.5f, 0.5f, 0.5f });
	}

	srand((unsigned int)time(NULL));

	// Creating cubes
	for (auto y = -2; y < 3; y++)
	{
		for (auto x = -2; x < 3; x++)
		{
			auto basicMat = getResourceManager().createResourceFromFile<dx3d::MaterialResource>((base/"DirectXGameEngine/Game/Assets/Shaders/Basic.hlsl").c_str());
			if (basicMat)
			{
				auto matData = dx3d::Vec3(1, 1, 1);
				basicMat->setData(std::as_bytes(std::span{ &matData, 1 }));
				basicMat->setTexture(0, woodTex);
			}

			auto cube = world.createGameObject<dx3d::GameObject>();
			auto comp = cube->createOrGetComponent<dx3d::MeshComponent>();
			comp->setMaterial(basicMat);
			comp->setMesh(cubeMesh);
			auto roty = (rand() % 628) / 100.0f;
			cube->getTransform().setScale({ 0.5,0.5,0.5 });
			cube->getTransform().setPosition({ x * 1.4f, 0.25f + 0.05f, y * 1.4f });
			cube->getTransform().setRotation({ 0,roty,0 });
			if (x == 0 && y == 0) m_testObject = cube; // add the cubes.
		}
	}

	// Creating Camera/Player
	int displayIndex = 0;
	for (auto& display : getDisplays())
	{
		auto camera = world.createGameObjectForWindow<Camera>(display->getID(), display->getInputSystem());
		camera->setName("Player Camera");
		auto* camComp = camera->createOrGetComponent<dx3d::CameraComponent>();
		display->setCamera(camComp);

		if (displayIndex == 0)
		{
			// Viewport 1 (Perspective, Lit)
			camera->getTransform().setPosition({ 0.0f, 1.0f, -2.0f });
			camera->getTransform().setRotation({ 0.0f, 0.0f, 0.0f });
			camComp->setProjectionMode(dx3d::ProjectionMode::Perspective);
			display->setRenderMode(dx3d::Display::RenderMode::Lit);
		}
		//else if (displayIndex == 1)
		//{
		//	// Viewport 2 (Top Down, Lit)
		//	camera->getTransform().setPosition({ 0.0f, 10.0f, 0.0f });
		//	camera->getTransform().setRotation({ 1.5708f, 0.0f, 0.0f });
		//	camComp->setProjectionMode(dx3d::ProjectionMode::Orthographic);
		//	display->setRenderMode(dx3d::Display::RenderMode::Lit);
		//}
		//else if (displayIndex == 2)
		//{
		//	// Viewport 3 (Perspective, Wireframe)
		//	camera->getTransform().setPosition({ 0.0f, 1.0f, -2.0f });
		//	camera->getTransform().setRotation({ 0.0f, 0.0f, 0.0f });
		//	camComp->setProjectionMode(dx3d::ProjectionMode::Perspective);
		//	display->setRenderMode(dx3d::Display::RenderMode::Wireframe);
		//}
		//displayIndex++;
	}
	/*auto player = world.createGameObject<Player>();
	player->getTransform().setPosition({ 0, 1, -2 });*/

	/*auto& display2 = getDisplays()[1];
	auto camera = world.createGameObjectForWindow<Camera>(display2->getID(), display2->getInputSystem());
	camera->getTransform().setPosition({ 0, 1, -2 });*/

	for (auto& display : getDisplays())
	{
		display->getInputSystem().setCursorLocked(false);
		display->getInputSystem().setCursorVisible(true);
		m_InspectorUIs[display->getID()] = std::make_unique<dx3d::InspectorUI>(dx3d::BaseDesc{ getLogger() });
	}
	
	// Adding world views
	onNewWorldView("Editor View");
	onNewWorldView("Game View");

	registerEditorEvents();
}

void MainGame::onUpdate(dx3d::f32 deltaTime)
{
	Game::onUpdate(deltaTime);

}

void MainGame::onDisplayAdded(dx3d::Display& display)
{
	onNewDisplay(display);
}

void MainGame::onDrawUi(dx3d::Display& display)
{
	/*if (m_testObject)
	{
		auto it = m_InspectorUIs.find(display.getID());
		if (it == m_InspectorUIs.end())
		{
			it = m_InspectorUIs.emplace(display.getID(), std::make_unique<dx3d::InspectorUI>(dx3d::BaseDesc{ getLogger() })).first;
		}
		it->second->draw(*m_testObject, display);
	}*/

	for (auto& m_UI : m_UIs)
	{
		m_UI->draw();
	}
}

void MainGame::registerEditorEvents()
{
	auto& events = dx3d::EventBroadcastManager::getInstance();

	events.addObserver(dx3d::EventNames::ON_ADD_EMPTY_GAMEOBJECT, [this]()
	{
		if (m_isPlayMode) return;

		auto* object = spawnEditorObject("Empty");
		if (!object) return;

		object->setDeleted(true);
		executeEditorCommand(EditorCommand{
			[object]() { object->setDeleted(true); },
			[object]() { object->setDeleted(false); }
		});
		selectGameObject(object);
	});

	events.addObserver(dx3d::EventNames::ON_ADD_3D_OBJECT, [this](dx3d::Parameters& params)
	{
		if (m_isPlayMode) return;

		auto type = params.GetStringExtra("Key", "Cube");
		if (type == "Obj")
		{
			const auto modelName = params.GetStringExtra("ModelName", "");
			if (modelName.empty()) return;
			type = "Obj:" + modelName;
		}

		auto* object = spawnEditorObject(type);
		if (!object) return;

		object->setDeleted(true);
		executeEditorCommand(EditorCommand{
			[object]() { object->setDeleted(true); },
			[object]() { object->setDeleted(false); }
		});
		selectGameObject(object);
	});

	events.addObserver(dx3d::EventNames::ON_DELETE_GAMEOBJECT, [this](dx3d::Parameters& params)
	{
			if (m_isPlayMode) return;

			auto* object = params.GetGameObjectPtr("Target", nullptr);
			if (!object || object->isDeleted()) return;

			//Collect all descendants recursively
			std::vector<dx3d::GameObject*> allAffected;
			std::function<void(dx3d::GameObject*)> collectDescendants = [&](dx3d::GameObject* obj)
				{
					if (!obj) return;
					allAffected.push_back(obj);
					for (auto* child : obj->getChildren())
					{
						if (child && !child->isDeleted())
						{
							collectDescendants(child);
						}
					}
				};
			collectDescendants(object);

			//Store old deleted states for all affected objects
			std::vector<bool> oldStates;
			for (auto* obj : allAffected)
			{
				oldStates.push_back(obj->isDeleted());
			}

			executeEditorCommand(EditorCommand{
				//Undo: restore all objects to their previous deleted state
				[allAffected, oldStates]()
				{
					for (size_t i = 0; i < allAffected.size(); i++)
					{
						if (allAffected[i])
						{
							allAffected[i]->setDeleted(oldStates[i]);
						}
					}
				},
				//Redo: delete all objects in the hierarchy
				[allAffected]()
				{
					for (auto* obj : allAffected)
					{
						if (obj)
						{
							obj->setDeleted(true);
						}
					}
				}
				});
			selectGameObject(nullptr);
	});

	events.addObserver(dx3d::EventNames::ON_SET_GAMEOBJECT_ENABLED, [this](dx3d::Parameters& params)
	{
			if (m_isPlayMode) return;

			auto* object = params.GetGameObjectPtr("Target", nullptr);
			if (!object || object->isDeleted()) return;

			const bool oldValue = object->isEnabled();
			const bool newValue = params.GetBoolExtra("Enabled", oldValue);
			if (oldValue == newValue) return;

			//Collect all descendants recursively
			std::vector<dx3d::GameObject*> allAffected;
			std::function<void(dx3d::GameObject*)> collectDescendants = [&](dx3d::GameObject* obj)
				{
					if (!obj) return;
					allAffected.push_back(obj);
					for (auto* child : obj->getChildren())
					{
						if (child && !child->isDeleted())
						{
							collectDescendants(child);
						}
					}
				};
			collectDescendants(object);

			//Store old enabled states for all affected objects
			std::vector<bool> oldStates;
			for (auto* obj : allAffected)
			{
				oldStates.push_back(obj->isEnabled());
			}

			executeEditorCommand(EditorCommand{
				//Undo: restore all objects to their previous enabled state
				[allAffected, oldStates]()
				{
					for (size_t i = 0; i < allAffected.size(); i++)
					{
						if (allAffected[i])
						{
							allAffected[i]->setEnabled(oldStates[i]);
						}
					}
				},
				//Redo: set all objects to the new enabled state
				[allAffected, newValue]()
				{
					for (auto* obj : allAffected)
					{
						if (obj)
						{
							obj->setEnabled(newValue);
						}
					}
				}
				});
	});

	events.addObserver(dx3d::EventNames::ON_SET_PHYSICS_ENABLED, [this](dx3d::Parameters& params)
	{
		if (m_isPlayMode) return;

		auto* object = params.GetGameObjectPtr("Target", nullptr);
		if (!object || object->isDeleted()) return;

		auto* physicsComponent = object->getComponent<dx3d::PhysicsComponent>();
		if (!physicsComponent) return;

		const bool oldValue = physicsComponent->isPhysicsEnabled();
		const bool newValue = params.GetBoolExtra("Enabled", oldValue);
		if (oldValue == newValue) return;

		executeEditorCommand(EditorCommand{
			[physicsComponent, oldValue]() { physicsComponent->setPhysicsEnabled(oldValue); },
			[physicsComponent, newValue]() { physicsComponent->setPhysicsEnabled(newValue); }
		});
	});

	events.addObserver(dx3d::EventNames::ON_TRANSFORM_CHANGED, [this](dx3d::Parameters& params)
	{
			if (m_isPlayMode) return;

			auto* object = params.GetGameObjectPtr("Target", nullptr);
			if (!object || object->isDeleted()) return;

			auto property = params.GetStringExtra("Property", "");
			auto oldValue = params.GetVec3Extra("OldValue", {});
			auto newValue = params.GetVec3Extra("NewValue", oldValue);

			auto applyValue = [object, property](const dx3d::Vec3& value)
				{
					if (property == "Position") object->getTransform().setPosition(value);
					else if (property == "Rotation") object->getTransform().setRotation(value);
					else if (property == "Scale") object->getTransform().setScale(value);

					// Sync physics if the object has a physics component
					if (auto* physComp = object->getComponent<dx3d::PhysicsComponent>())
					{
						physComp->syncTransformToPhysics();
					}
				};

			executeEditorCommand(EditorCommand{
				[applyValue, oldValue]() { applyValue(oldValue); },
				[applyValue, newValue]() { applyValue(newValue); }
				});
	});

	events.addObserver(dx3d::EventNames::ON_SET_PARENT, [this](dx3d::Parameters& params)
	{
		if (m_isPlayMode) return;

		auto* child = params.GetGameObjectPtr("Child", nullptr);
		auto* newParent = params.GetGameObjectPtr("Parent", nullptr);

		if (!child || child->isDeleted()) return;
		if (child == newParent) return;

		auto* oldParent = child->getParent();
		if (oldParent == newParent) return;

		executeEditorCommand(EditorCommand{
			[child, oldParent]() { child->setParent(oldParent); },
			[child, newParent]() { child->setParent(newParent); }
			});
	});

	events.addObserver(dx3d::EventNames::ON_EDITOR_UNDO, [this]() { undoEditorCommand(); });
	events.addObserver(dx3d::EventNames::ON_EDITOR_REDO, [this]() { redoEditorCommand(); });
	events.addObserver(dx3d::EventNames::ON_SCENE_SAVE, [this]()
	{
		const auto scenePath = getDefaultScenePath();
		if (saveSceneToFile(scenePath))
		{
			getLogger().log(dx3d::Logger::LogLevel::Info, "Scene saved to {}", scenePath.string());
		}
		else
		{
			getLogger().log(dx3d::Logger::LogLevel::Warning, "Failed to save scene to {}", scenePath.string());
		}
	});
	events.addObserver(dx3d::EventNames::ON_SCENE_SAVE_AS_NEW, [this]()
	{
		const auto scenePath = getNewScenePath();
		if (saveSceneToFile(scenePath))
		{
			getLogger().log(dx3d::Logger::LogLevel::Info, "Scene saved as new file: {}", scenePath.string());
		}
		else
		{
			getLogger().log(dx3d::Logger::LogLevel::Warning, "Failed to save new scene to {}", scenePath.string());
		}
	});
	events.addObserver(dx3d::EventNames::ON_SCENE_LOAD, [this](dx3d::Parameters& params)
	{
		const auto scenePath = std::filesystem::path(params.GetStringExtra("Path", getDefaultScenePath().string()));
		if (loadSceneFromFile(scenePath))
		{
			getLogger().log(dx3d::Logger::LogLevel::Info, "Scene loaded from {}", scenePath.string());
		}
		else
		{
			getLogger().log(dx3d::Logger::LogLevel::Warning, "Failed to load scene from {}", scenePath.string());
		}
	});
	events.addObserver(dx3d::EventNames::ON_EDITOR_PLAY_MODE_CHANGED, [this](dx3d::Parameters& params)
	{
		setPlayMode(params.GetBoolExtra("IsPlayMode", false));
	});
}

void MainGame::executeEditorCommand(EditorCommand command)
{
	if (m_isPlayMode) return;

	command.redo();
	m_undoStack.push_back(command);
	if (m_undoStack.size() > MaxUndoCommands)
	{
		m_undoStack.erase(m_undoStack.begin());
	}
	m_redoStack.clear();
}

void MainGame::undoEditorCommand()
{
	if (m_isPlayMode || m_undoStack.empty()) return;

	auto command = m_undoStack.back();
	m_undoStack.pop_back();
	command.undo();
	m_redoStack.push_back(command);
}

void MainGame::redoEditorCommand()
{
	if (m_isPlayMode || m_redoStack.empty()) return;

	auto command = m_redoStack.back();
	m_redoStack.pop_back();
	command.redo();
	m_undoStack.push_back(command);
}

void MainGame::setPlayMode(bool isPlayMode)
{
	m_isPlayMode = isPlayMode;
}

dx3d::GameObject* MainGame::spawnEditorObject(const std::string& type)
{
	auto* object = getWorld().createGameObject<dx3d::GameObject>();
	if (!object) return nullptr;

	bool isPhysics = (type.rfind("Physics-", 0) == 0);
	std::string cleanType = type;
	if (isPhysics)
	{
		cleanType = type.substr(8); // Remove "Physics-" prefix
	}

	object->getTransform().setPosition({ 0.0f, 0.5f, 0.0f });
	object->getTransform().setScale({ 0.5f, 0.5f, 0.5f });

	dx3d::RefPtr<dx3d::Mesh> spawnMesh{};
	std::string displayName = cleanType == "Empty" ? std::string{ "Empty GameObject" } : cleanType;

	dx3d::PhysicsColliderType colliderType = dx3d::PhysicsColliderType::Box;
	dx3d::Vec3 colliderSize = { 1.0f, 1.0f, 1.0f };

	if (cleanType == "Cube")
	{
		spawnMesh = m_spawnCubeMesh;
		colliderType = dx3d::PhysicsColliderType::Box;
		colliderSize = { 1.0f, 1.0f, 1.0f };
	}
	else if (cleanType == "Sphere")
	{
		spawnMesh = m_spawnSphereMesh;
		colliderType = dx3d::PhysicsColliderType::Sphere;
		colliderSize = { 1.0f, 1.0f, 1.0f };
	}
	else if (cleanType == "Capsule")
	{
		spawnMesh = m_spawnCapsuleMesh;
		colliderType = dx3d::PhysicsColliderType::Capsule;
		colliderSize = { 1.0f, 2.0f, 1.0f };
	}
	else if (cleanType == "Cylinder")
	{
		spawnMesh = m_spawnCylinderMesh;
		colliderType = dx3d::PhysicsColliderType::Capsule;
		colliderSize = { 1.0f, 2.0f, 1.0f };
	}
	else if (cleanType == "Plane")
	{
		spawnMesh = m_spawnPlaneMesh;
		colliderType = dx3d::PhysicsColliderType::Box;
		colliderSize = { 10.0f, 0.1f, 10.0f };
	}
	else if (cleanType.rfind("Obj:", 0) == 0)
	{
		const auto modelName = cleanType.substr(4);
		spawnMesh = getMeshFactory().getCustomMesh(modelName);
		if (!modelName.empty())
		{
			displayName = modelName;
		}
	}

	if (cleanType.rfind("Obj:", 0) == 0 && !spawnMesh)
	{
		object->setDeleted(true);
		return nullptr;
	}

	if (isPhysics)
	{
		displayName = "Physics-" + displayName;
	}

	const auto objectIndex = ++m_spawnedObjectCounters[type];
	object->setName(displayName + " " + std::to_string(objectIndex));

	if (spawnMesh && m_spawnMaterial)
	{
		auto* mesh = object->createOrGetComponent<dx3d::MeshComponent>();
		mesh->setMesh(spawnMesh);
		mesh->setMaterial(m_spawnMaterial);
	}

	//Add physics component if it's a physics object
	if (isPhysics && spawnMesh)
	{
		auto* physComp = object->createOrGetComponent<dx3d::PhysicsComponent>();

		physComp->setColliderType(colliderType);
		physComp->setColliderSize(colliderSize);

		if (cleanType == "Plane")
		{
			physComp->setBodyType(dx3d::PhysicsBodyType::Static);
			physComp->setMass(0.0f);
		}
		else
		{
			physComp->setBodyType(dx3d::PhysicsBodyType::Dynamic);
			physComp->setMass(1.0f);
		}
		physComp->setUseGravity(true);
		physComp->initialize();
	}

	return object;
}

void MainGame::selectGameObject(dx3d::GameObject* object)
{
	dx3d::Parameters params;
	params.PutExtra("Selected", object);
	dx3d::EventBroadcastManager::getInstance().postEvent(dx3d::EventNames::ON_GAMEOBJECT_SELECTED, params);
}

std::filesystem::path MainGame::getDefaultScenePath() const
{
	return std::filesystem::current_path() / "Assets" / "Scenes" / "editor_scene.json";
}

std::filesystem::path MainGame::getNewScenePath() const
{
	const auto scenesDirectory = std::filesystem::current_path() / "Assets" / "Scenes";

	for (size_t index = 1; index < 10000; ++index)
	{
		auto candidate = scenesDirectory / ("scene_" + std::to_string(index) + ".json");
		std::error_code error;
		if (!std::filesystem::exists(candidate, error))
		{
			return candidate;
		}
	}

	return scenesDirectory / "scene_new.json";
}

bool MainGame::saveSceneToFile(const std::filesystem::path& path)
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

	for (const auto& [typeId, objects] : getWorld().getGameObjectList())
	{
		for (const auto& objectPtr : objects)
		{
			auto* object = objectPtr.get();
			if (!object || object->isDeleted()) continue;
			if (object->getComponent<dx3d::CameraComponent>()) continue;

			const auto objectType = getSerializableObjectType(*object);
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

			if (wroteObject)
			{
				file << ",\n";
			}

			file << "    {\n"
				<< "      \"id\": " << object->getID() << ",\n"
				<< "      \"parentId\": " << parentId << ",\n"
				<< "      \"name\": \"" << escapeJsonString(object->getName()) << "\",\n"
				<< "      \"type\": \"" << escapeJsonString(objectType) << "\",\n"
				<< "      \"enabled\": " << (object->isEnabled() ? "true" : "false") << ",\n"
				<< "      \"physicsEnabled\": " << ((physics && physics->isPhysicsEnabled()) ? "true" : "false") << ",\n"
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

bool MainGame::loadSceneFromFile(const std::filesystem::path& path)
{
	std::ifstream file(path);
	if (!file) return false;

	std::vector<SerializableSceneObjectData> sceneObjects;
	std::string line;
	while (std::getline(file, line))
	{
		if (line.find("\"objects\"") == std::string::npos) continue;

		while (true)
		{
			SerializableSceneObjectData data;
			const auto beforeRead = file.tellg();
			if (!readSceneObjectFromJson(file, data)) break;
			if (data.name.empty() || data.type.empty()) return false;
			sceneObjects.push_back(data);
			if (beforeRead == file.tellg()) break;
		}
		break;
	}

	clearSerializableSceneObjects();

	std::unordered_map<size_t, dx3d::GameObject*> loadedObjectsBySavedId;
	std::vector<std::pair<SerializableSceneObjectData, dx3d::GameObject*>> loadedObjects;

	for (const auto& data : sceneObjects)
	{
		auto* object = spawnEditorObject(data.type);
		if (!object) continue;

		object->setName(data.name);
		object->setEnabled(data.enabled);
		if (data.id != 0)
		{
			loadedObjectsBySavedId[data.id] = object;
		}
		loadedObjects.push_back({ data, object });
	}

	for (const auto& [data, object] : loadedObjects)
	{
		if (!object || data.parentId == 0) continue;

		auto parentIt = loadedObjectsBySavedId.find(data.parentId);
		if (parentIt != loadedObjectsBySavedId.end())
		{
			object->setParent(parentIt->second);
		}
	}

	for (const auto& [data, object] : loadedObjects)
	{
		if (!object) continue;

		object->getTransform().setPosition(data.position);
		object->getTransform().setRotation(data.rotation);
		object->getTransform().setScale(data.scale);

		if (auto* physics = object->getComponent<dx3d::PhysicsComponent>())
		{
			physics->syncTransformToPhysics();
			physics->setPhysicsEnabled(data.physicsEnabled);
		}
	}

	m_undoStack.clear();
	m_redoStack.clear();
	selectGameObject(nullptr);

	return true;
}

std::string MainGame::getSerializableObjectType(dx3d::GameObject& object)
{
	auto* meshComponent = object.getComponent<dx3d::MeshComponent>();
	auto* physicsComponent = object.getComponent<dx3d::PhysicsComponent>();
	const auto prefix = physicsComponent ? std::string{ "Physics-" } : std::string{};

	if (!meshComponent || !meshComponent->getMesh())
	{
		return "Empty";
	}

	const auto& mesh = meshComponent->getMesh();
	if (mesh == m_spawnCubeMesh) return prefix + "Cube";
	if (mesh == m_spawnSphereMesh) return prefix + "Sphere";
	if (mesh == m_spawnCapsuleMesh) return prefix + "Capsule";
	if (mesh == m_spawnCylinderMesh) return prefix + "Cylinder";
	if (mesh == m_spawnPlaneMesh) return prefix + "Plane";

	for (const auto& modelName : m_availableObjModels)
	{
		if (mesh == getMeshFactory().getCustomMesh(modelName))
		{
			return "Obj:" + modelName;
		}
	}

	return {};
}

void MainGame::clearSerializableSceneObjects()
{
	for (const auto& [typeId, objects] : getWorld().getGameObjectList())
	{
		for (const auto& objectPtr : objects)
		{
			auto* object = objectPtr.get();
			if (!object || object->isDeleted()) continue;
			if (object->getComponent<dx3d::CameraComponent>()) continue;

			if (auto* physics = object->getComponent<dx3d::PhysicsComponent>())
			{
				physics->setPhysicsEnabled(false);
			}
			object->setDeleted(true);
		}
	}
}
