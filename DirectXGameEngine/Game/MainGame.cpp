#include "MainGame.h"
#include "Objects/Player.h"
#include "Objects/Camera.h"
#include <DX3D/Graphics/Mesh/MeshFactory.h>
#include <DX3D/Component/MeshComponent.h>
#include <DX3D/Component/CameraComponent.h>
#include <filesystem>

#include <DX3D/UI/DebugWindowUI.h>
#include <DX3D/UI/HierarchyUI.h>
#include <DX3D/UI/MainMenuBarUI.h>

#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>
#include <DX3D/EventBroadcasting/Parameters.h>

#include <DX3D/Resource/TextureManager.h>

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
	std::unique_ptr<dx3d::HierarchyUI> hierarchy_UI = std::make_unique<dx3d::HierarchyUI>(dx3d::BaseDesc{ getLogger() });
	hierarchy_UI->setGameObjectList(&world.getGameObjectList());
	m_UIs.push_back(std::move(hierarchy_UI));
	m_UIs.push_back(std::make_unique<dx3d::DebugWindowUI>(dx3d::BaseDesc{ getLogger() }));

	m_UIs.push_back(std::make_unique<dx3d::MainMenuBarUI>(dx3d::BaseDesc{ getLogger() }));
	m_UIs.push_back(std::make_unique<dx3d::InspectorUI>(dx3d::BaseDesc{ getLogger() }));

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

		executeEditorCommand(EditorCommand{
			[object]() { object->setDeleted(false); },
			[object]() { object->setDeleted(true); }
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

		executeEditorCommand(EditorCommand{
			[object, oldValue]() { object->setEnabled(oldValue); },
			[object, newValue]() { object->setEnabled(newValue); }
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

	const auto objectTypeName = type == "Empty" ? std::string{ "Empty GameObject" } : type;
	const auto objectIndex = ++m_spawnedObjectCounters[objectTypeName];
	object->setName(objectTypeName + " " + std::to_string(objectIndex));

	object->getTransform().setPosition({ 0.0f, 0.5f, 0.0f });
	object->getTransform().setScale({ 0.5f, 0.5f, 0.5f });

	dx3d::RefPtr<dx3d::Mesh> spawnMesh{};
	if (type == "Cube") spawnMesh = m_spawnCubeMesh;
	else if (type == "Sphere") spawnMesh = m_spawnSphereMesh;
	else if (type == "Capsule") spawnMesh = m_spawnCapsuleMesh;
	else if (type == "Cylinder") spawnMesh = m_spawnCylinderMesh;
	else if (type == "Plane") spawnMesh = m_spawnPlaneMesh;

	if (spawnMesh && m_spawnMaterial)
	{
		auto* mesh = object->createOrGetComponent<dx3d::MeshComponent>();
		mesh->setMesh(spawnMesh);
		mesh->setMaterial(m_spawnMaterial);
	}

	return object;
}

void MainGame::selectGameObject(dx3d::GameObject* object)
{
	dx3d::Parameters params;
	params.PutExtra("Selected", object);
	dx3d::EventBroadcastManager::getInstance().postEvent(dx3d::EventNames::ON_GAMEOBJECT_SELECTED, params);
}
