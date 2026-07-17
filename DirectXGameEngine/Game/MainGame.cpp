#include "MainGame.h"
#include "Objects/Player.h"
#include "Objects/Camera.h"
#include <DX3D/Graphics/Mesh/MeshFactory.h>
#include <DX3D/Component/MeshComponent.h>
#include <DX3D/Component/CameraComponent.h>
#include <filesystem>

#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>

MainGame::MainGame(const dx3d::GameDesc& desc) : dx3d::Game(desc)
{
}

void MainGame::onNewDisplay(dx3d::Display& display)
{
	auto& world = getWorld();

	auto camera = world.createGameObjectForWindow<Camera>(display.getID(), display.getInputSystem());
	auto* camComp = camera->createOrGetComponent<dx3d::CameraComponent>();
	display.setCamera(camComp);

	camera->getTransform().setPosition({ 0.0f, 1.0f, -2.0f });
	camera->getTransform().setRotation({ 0.0f, 0.0f, 0.0f });
	camComp->setProjectionMode(dx3d::ProjectionMode::Perspective);
	display.setRenderMode(dx3d::Display::RenderMode::Lit);

	display.getInputSystem().setCursorLocked(false);
	display.getInputSystem().setCursorVisible(true);

	m_InspectorUIs[display.getID()] = std::make_unique<dx3d::InspectorUI>(dx3d::BaseDesc{ getLogger() });
}

void MainGame::onCreate()
{
	Game::onCreate();
	auto& world = getWorld();
	std::filesystem::path base = std::filesystem::current_path().parent_path();
	auto woodTex = getResourceManager().createResourceFromFile<dx3d::TextureResource>((base/"DirectXGameEngine/Game/Assets/Textures/wood.jpg").c_str());
	auto floorTex = getResourceManager().createResourceFromFile<dx3d::TextureResource>((base / "DirectXGameEngine/Game/Assets/Textures/floor.jpg").c_str());

	m_MainMenuBarUI = std::make_unique<dx3d::MainMenuBarUI>(dx3d::BaseDesc{ getLogger() });
	
	
	// Create mesh resources (reusable)
	auto cubeMesh = dx3d::MeshFactory::createCubeMesh();
	auto sphereMesh = dx3d::MeshFactory::createSphereMesh(20, 20);
	auto capsuleMesh = dx3d::MeshFactory::createCapsuleMesh(0.5f, 2.0f);
	auto cylinderMesh = dx3d::MeshFactory::createCylinderMesh(0.5f, 2.0f);
	auto planeMesh = dx3d::MeshFactory::createPlaneMesh(10.0f, 10.0f);
	auto circleMesh = dx3d::MeshFactory::createCircleMesh(0.5f, 32);

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
		floor->getTransform().setScale({ 6.8f, 0.1f, 6.8f });
		floor->getTransform().setPosition({ 0, 0, 0 });

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
		else if (displayIndex == 1)
		{
			// Viewport 2 (Top Down, Lit)
			camera->getTransform().setPosition({ 0.0f, 10.0f, 0.0f });
			camera->getTransform().setRotation({ 1.5708f, 0.0f, 0.0f });
			camComp->setProjectionMode(dx3d::ProjectionMode::Orthographic);
			display->setRenderMode(dx3d::Display::RenderMode::Lit);
		}
		else if (displayIndex == 2)
		{
			// Viewport 3 (Perspective, Wireframe)
			camera->getTransform().setPosition({ 0.0f, 1.0f, -2.0f });
			camera->getTransform().setRotation({ 0.0f, 0.0f, 0.0f });
			camComp->setProjectionMode(dx3d::ProjectionMode::Perspective);
			display->setRenderMode(dx3d::Display::RenderMode::Wireframe);
		}
		displayIndex++;
	}
	auto player = world.createGameObject<Player>();
	player->getTransform().setPosition({ 0, 1, -2 });

	/*auto& display2 = getDisplays()[1];
	auto camera = world.createGameObjectForWindow<Camera>(display2->getID(), display2->getInputSystem());
	camera->getTransform().setPosition({ 0, 1, -2 });*/

	for (auto& display : getDisplays())
	{
		display->getInputSystem().setCursorLocked(false);
		display->getInputSystem().setCursorVisible(true);
		m_InspectorUIs[display->getID()] = std::make_unique<dx3d::InspectorUI>(dx3d::BaseDesc{ getLogger() });
	}

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
	if (m_testObject)
	{
		auto it = m_InspectorUIs.find(display.getID());
		if (it == m_InspectorUIs.end())
		{
			it = m_InspectorUIs.emplace(display.getID(), std::make_unique<dx3d::InspectorUI>(dx3d::BaseDesc{ getLogger() })).first;
		}
		it->second->draw(*m_testObject, display);
	}

	m_MainMenuBarUI->draw();
}
