#include "MainGame.h"
#include "SceneSerializer.h"
#include "Objects/Player.h"
#include "Objects/Camera.h"
#include <DX3D/Graphics/Mesh/MeshFactory.h>
#include <DX3D/Component/MeshComponent.h>
#include <DX3D/Component/CameraComponent.h>
#include <DX3D/Component/PointLightComponent.h>
#include <filesystem>
#include <functional>
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

	/*dx3d::PhysicsColliderType colliderType = dx3d::PhysicsColliderType::Box;
	dx3d::Vec3 colliderSize = { 1.0f, 1.0f, 1.0f };*/

	// Creating cubes
	//for (auto z = -2; z < 3; z++)
	//{
	//	for (auto y = -2; y < 3; y++)
	//	{
	//		for (auto x = -2; x < 3; x++)
	//		{
	//			auto basicMat = getResourceManager().createResourceFromFile<dx3d::MaterialResource>((base / "DirectXGameEngine/Game/Assets/Shaders/Basic.hlsl").c_str());
	//			if (basicMat)
	//			{
	//				auto matData = dx3d::Vec3(1, 1, 1);
	//				basicMat->setData(std::as_bytes(std::span{ &matData, 1 }));
	//				basicMat->setTexture(0, woodTex);
	//			}

	//			auto cube = world.createGameObject<dx3d::GameObject>();
	//			auto comp = cube->createOrGetComponent<dx3d::MeshComponent>();
	//			comp->setMaterial(basicMat);
	//			comp->setMesh(cubeMesh);


	//			//Add physics component if it's a physics object

	//			auto* physComp = cube->createOrGetComponent<dx3d::PhysicsComponent>();

	//			physComp->setColliderType(colliderType);
	//			physComp->setColliderSize(colliderSize);

	//			physComp->setBodyType(dx3d::PhysicsBodyType::Dynamic);
	//			physComp->setMass(1.0f);

	//			physComp->setUseGravity(true);
	//			physComp->initialize();

	//			auto roty = (rand() % 628) / 100.0f;
	//			cube->getTransform().setScale({ 0.5,0.5,0.5 });
	//			cube->getTransform().setPosition({ x * 1.4f, y * 1.4f, z * 1.4f });
	//			cube->getTransform().setRotation({ 0,roty,0 });


	//			if (x == 0 && y == 0) m_testObject = cube; // add the cubes.
	//		}
	//	}
	//}

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

	events.addObserver(dx3d::EventNames::ON_ADD_RIGID_BODY, [this](dx3d::Parameters& params)
	{
		if (m_isPlayMode) return;

		auto* object = params.GetGameObjectPtr("Target", nullptr);
		if (!object || object->isDeleted()) return;
		if (object->getComponent<dx3d::PhysicsComponent>()) return;
		if (object->getComponent<dx3d::CameraComponent>()) return;
		if (object->getComponent<dx3d::PointLightComponent>()) return;

		executeEditorCommand(EditorCommand{
			[this, object]() { removeRigidBodyComponent(*object); },
			[this, object]() { addRigidBodyComponent(*object); }
		});
	});

	events.addObserver(dx3d::EventNames::ON_REMOVE_RIGID_BODY, [this](dx3d::Parameters& params)
	{
		if (m_isPlayMode) return;

		auto* object = params.GetGameObjectPtr("Target", nullptr);
		if (!object || object->isDeleted()) return;

		auto* physicsComponent = object->getComponent<dx3d::PhysicsComponent>();
		if (!physicsComponent) return;

		const auto bodyType = physicsComponent->getBodyType();
		const auto colliderType = physicsComponent->getColliderType();
		const auto colliderSize = physicsComponent->getColliderSize();
		const auto mass = physicsComponent->getMass();
		const auto useGravity = physicsComponent->isUseGravityEnabled();
		const auto physicsEnabled = physicsComponent->isPhysicsEnabled();

		executeEditorCommand(EditorCommand{
			[this, object, bodyType, colliderType, colliderSize, mass, useGravity, physicsEnabled]()
			{
				auto* restoredPhysics = addRigidBodyComponent(*object);
				if (!restoredPhysics) return;
				restoredPhysics->setBodyType(bodyType);
				restoredPhysics->setColliderType(colliderType);
				restoredPhysics->setColliderSize(colliderSize);
				restoredPhysics->setMass(mass);
				restoredPhysics->setUseGravity(useGravity);
				restoredPhysics->setPhysicsEnabled(physicsEnabled);
			},
			[this, object]() { removeRigidBodyComponent(*object); }
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
		const auto scenePath = m_currentScenePath.empty() ? getDefaultScenePath() : m_currentScenePath;
		if (saveSceneToFile(scenePath))
		{
			m_currentScenePath = scenePath;
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
			m_currentScenePath = scenePath;
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
			m_currentScenePath = scenePath;
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

	if (cleanType == "Cube")
	{
		spawnMesh = m_spawnCubeMesh;
	}
	else if (cleanType == "Sphere")
	{
		spawnMesh = m_spawnSphereMesh;
	}
	else if (cleanType == "Capsule")
	{
		spawnMesh = m_spawnCapsuleMesh;
	}
	else if (cleanType == "Cylinder")
	{
		spawnMesh = m_spawnCylinderMesh;
	}
	else if (cleanType == "Plane")
	{
		spawnMesh = m_spawnPlaneMesh;
	}
	else if (cleanType == "Point Light")
	{
		spawnMesh = m_spawnSphereMesh;
		object->getTransform().setPosition({ 0.0f, 2.0f, 0.0f });
		object->getTransform().setScale({ 0.2f, 0.2f, 0.2f });
		auto* pointLight = object->createOrGetComponent<dx3d::PointLightComponent>();
		pointLight->setColor({ 1.0f, 0.95f, 0.85f });
		pointLight->setIntensity(1.5f);
		pointLight->setRange(8.0f);
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

	if (spawnMesh)
	{
		auto* mesh = object->createOrGetComponent<dx3d::MeshComponent>();
		mesh->setMesh(spawnMesh);
		mesh->setMaterial(createEditorMaterial());
	}

	//Add physics component if it's a physics object
	if (isPhysics && spawnMesh)
	{
		addRigidBodyComponent(*object);
	}

	return object;
}

dx3d::RefPtr<dx3d::MaterialResource> MainGame::createEditorMaterial(const std::string& textureName)
{
	std::filesystem::path base = std::filesystem::current_path().parent_path();
	auto material = getResourceManager().createResourceFromFile<dx3d::MaterialResource>((base / "DirectXGameEngine/Game/Assets/Shaders/Basic.hlsl").c_str());
	if (!material) return {};

	auto matData = dx3d::Vec3(1, 1, 1);
	material->setData(std::as_bytes(std::span{ &matData, 1 }));

	auto texture = dx3d::TextureManager::getInstance().getTexture(textureName);
	if (texture)
	{
		material->setTexture(0, texture);
	}

	return material;
}

dx3d::PhysicsComponent* MainGame::addRigidBodyComponent(dx3d::GameObject& object)
{
	if (object.isDeleted()) return nullptr;
	if (object.getComponent<dx3d::CameraComponent>()) return nullptr;
	if (object.getComponent<dx3d::PointLightComponent>()) return nullptr;

	auto* physicsComponent = object.createOrGetComponent<dx3d::PhysicsComponent>();
	if (!physicsComponent) return nullptr;

	configureRigidBodyForObject(object, *physicsComponent);
	physicsComponent->setPhysicsEnabled(true);
	physicsComponent->initialize();
	physicsComponent->syncTransformToPhysics();

	return physicsComponent;
}

void MainGame::removeRigidBodyComponent(dx3d::GameObject& object)
{
	if (auto* physicsComponent = object.getComponent<dx3d::PhysicsComponent>())
	{
		physicsComponent->setPhysicsEnabled(false);
	}
	object.removeComponent<dx3d::PhysicsComponent>();
}

void MainGame::configureRigidBodyForObject(dx3d::GameObject& object, dx3d::PhysicsComponent& physicsComponent)
{
	auto colliderType = dx3d::PhysicsColliderType::Box;
	auto colliderSize = dx3d::Vec3{ 1.0f, 1.0f, 1.0f };
	auto bodyType = dx3d::PhysicsBodyType::Dynamic;
	auto mass = 1.0f;

	if (auto* meshComponent = object.getComponent<dx3d::MeshComponent>())
	{
		const auto& mesh = meshComponent->getMesh();
		if (mesh == m_spawnSphereMesh)
		{
			colliderType = dx3d::PhysicsColliderType::Sphere;
		}
		else if (mesh == m_spawnCapsuleMesh || mesh == m_spawnCylinderMesh)
		{
			colliderType = dx3d::PhysicsColliderType::Capsule;
			colliderSize = { 1.0f, 2.0f, 1.0f };
		}
		else if (mesh == m_spawnPlaneMesh)
		{
			colliderType = dx3d::PhysicsColliderType::Box;
			colliderSize = { 10.0f, 0.1f, 10.0f };
			bodyType = dx3d::PhysicsBodyType::Static;
			mass = 0.0f;
		}
	}

	physicsComponent.setColliderType(colliderType);
	physicsComponent.setColliderSize(colliderSize);
	physicsComponent.setBodyType(bodyType);
	physicsComponent.setMass(mass);
	physicsComponent.setUseGravity(bodyType == dx3d::PhysicsBodyType::Dynamic);
}

void MainGame::selectGameObject(dx3d::GameObject* object)
{
	dx3d::Parameters params;
	params.PutExtra("Selected", object);
	dx3d::EventBroadcastManager::getInstance().postEvent(dx3d::EventNames::ON_GAMEOBJECT_SELECTED, params);
}

std::filesystem::path MainGame::getDefaultScenePath() const
{
	return std::filesystem::current_path() / "Assets" / "Scenes" / "editor_scene.level";
}

std::filesystem::path MainGame::getNewScenePath() const
{
	const auto scenesDirectory = std::filesystem::current_path() / "Assets" / "Scenes";

	for (size_t index = 1; index < 10000; ++index)
	{
		auto candidate = scenesDirectory / ("scene_" + std::to_string(index) + ".level");
		std::error_code error;
		if (!std::filesystem::exists(candidate, error))
		{
			return candidate;
		}
	}

	return scenesDirectory / "scene_new.level";
}

bool MainGame::saveSceneToFile(const std::filesystem::path& path)
{
	return SceneSerializer::saveToJsonFile(
		path,
		getWorld(),
		[this](dx3d::GameObject& object)
		{
			return getSerializableObjectType(object);
		});
}

bool MainGame::loadSceneFromFile(const std::filesystem::path& path)
{
	std::vector<SceneObjectData> sceneObjects;
	if (!SceneSerializer::loadFromJsonFile(path, sceneObjects)) return false;

	clearSerializableSceneObjects();

	std::unordered_map<size_t, dx3d::GameObject*> loadedObjectsBySavedId;
	std::vector<std::pair<SceneObjectData, dx3d::GameObject*>> loadedObjects;

	for (const auto& data : sceneObjects)
	{
		auto* object = spawnEditorObject(data.type);
		if (!object) continue;

		object->setName(data.name);
		object->setEnabled(data.enabled);
		const bool shouldAttachRigidBody = data.hasRigidBody || data.physicsEnabled || data.type.rfind("Physics-", 0) == 0;
		if (shouldAttachRigidBody && !object->getComponent<dx3d::PhysicsComponent>())
		{
			addRigidBodyComponent(*object);
		}
		if (shouldAttachRigidBody)
		{
			if (auto* physics = object->getComponent<dx3d::PhysicsComponent>())
			{
				physics->setPhysicsEnabled(false);
			}
		}
		if (!data.textureName.empty())
		{
			if (auto* mesh = object->getComponent<dx3d::MeshComponent>())
			{
				mesh->setMaterial(createEditorMaterial(data.textureName));
			}
		}
		if (auto* pointLight = object->getComponent<dx3d::PointLightComponent>())
		{
			if (data.pointLightIntensity > 0.0f)
			{
				pointLight->setIntensity(data.pointLightIntensity);
			}
			if (data.pointLightRange > 0.0f)
			{
				pointLight->setRange(data.pointLightRange);
			}
		}
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
			if (data.rigidBodyType == "Static") physics->setBodyType(dx3d::PhysicsBodyType::Static);
			else if (data.rigidBodyType == "Kinematic") physics->setBodyType(dx3d::PhysicsBodyType::Kinematic);
			else if (data.hasRigidBody || data.physicsEnabled) physics->setBodyType(dx3d::PhysicsBodyType::Dynamic);

			if (data.rigidBodyCollider == "Sphere") physics->setColliderType(dx3d::PhysicsColliderType::Sphere);
			else if (data.rigidBodyCollider == "Capsule") physics->setColliderType(dx3d::PhysicsColliderType::Capsule);
			else if (data.rigidBodyCollider == "ConvexMesh") physics->setColliderType(dx3d::PhysicsColliderType::ConvexMesh);
			else if (data.rigidBodyCollider == "ConcaveMesh") physics->setColliderType(dx3d::PhysicsColliderType::ConcaveMesh);
			else if (!data.rigidBodyCollider.empty()) physics->setColliderType(dx3d::PhysicsColliderType::Box);

			if (data.rigidBodyColliderSize.x > 0.0f || data.rigidBodyColliderSize.y > 0.0f || data.rigidBodyColliderSize.z > 0.0f)
			{
				physics->setColliderSize(data.rigidBodyColliderSize);
			}
			if (data.rigidBodyMass > 0.0f || data.rigidBodyType == "Static")
			{
				physics->setMass(data.rigidBodyMass);
			}
			physics->setUseGravity(data.rigidBodyUseGravity);
			const bool restoredPhysicsEnabled = data.rigidBodyEnabledSpecified ? data.physicsEnabled : (data.hasRigidBody || data.physicsEnabled);
			physics->setPhysicsEnabled(restoredPhysicsEnabled);
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

	if (object.getComponent<dx3d::PointLightComponent>())
	{
		return "Point Light";
	}

	if (!meshComponent || !meshComponent->getMesh())
	{
		return "Empty";
	}

	const auto& mesh = meshComponent->getMesh();
	if (mesh == m_spawnCubeMesh) return "Cube";
	if (mesh == m_spawnSphereMesh) return "Sphere";
	if (mesh == m_spawnCapsuleMesh) return "Capsule";
	if (mesh == m_spawnCylinderMesh) return "Cylinder";
	if (mesh == m_spawnPlaneMesh) return "Plane";

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
