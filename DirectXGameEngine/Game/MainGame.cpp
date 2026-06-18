#include "MainGame.h"
#include "Objects/Player.h"
#include <DX3D/Graphics/MeshFactory.h>
#include <DX3D/Component/MeshComponent.h>


MainGame::MainGame(const dx3d::GameDesc& desc) : dx3d::Game(desc)
{
}

void MainGame::onCreate()
{
	Game::onCreate();
	auto& world = getWorld();

	// Create mesh resources (reusable)
	auto cubeMesh = dx3d::MeshFactory::createCubeMesh();
	auto sphereMesh = dx3d::MeshFactory::createSphereMesh(20, 20);
	auto capsuleMesh = dx3d::MeshFactory::createCapsuleMesh(0.5f, 2.0f);
	auto cylinderMesh = dx3d::MeshFactory::createCylinderMesh(0.5f, 2.0f);
	auto planeMesh = dx3d::MeshFactory::createPlaneMesh(10.0f, 10.0f);
	auto circleMesh = dx3d::MeshFactory::createCircleMesh(0.5f, 32);

	// Create a floor with plane
	auto floor = world.createGameObject<dx3d::GameObject>();
	auto floorMeshComp = floor->createOrGetComponent<dx3d::MeshComponent>();
	floorMeshComp->setMesh(planeMesh);
	floor->getTransform().setScale({ 1.0f, 0.0f, 1.0f });
	floor->getTransform().setPosition({ 0.0f, -1.0f, 0.0f });

	srand((unsigned int)time(NULL));

	// Create cubes
	/*for (auto y = -2; y < 3; y++)
	{
		for (auto x = -2; x < 3; x++)
		{
			auto cube = world.createGameObject<dx3d::GameObject>();
			auto cube_meshComponent = cube->createOrGetComponent<dx3d::MeshComponent>();
			cube_meshComponent->setMesh(cubeMesh);
			auto height = (rand() % 120) + (80.0f);
			height /= 100.0f;

			auto width = (rand() % 600) + (200.0f);
			width /= 1000.0f;

			cube->getTransform().setScale({ width, height, width });
			cube->getTransform().setPosition({ x * 1.4f, (height / 2.0f) - 1.0f, y * 1.4f });
		}
	}*/

	// Create a capsule
	auto capsule = world.createGameObject<dx3d::GameObject>();
	auto capsuleMeshComp = capsule->createOrGetComponent<dx3d::MeshComponent>();
	capsuleMeshComp->setMesh(capsuleMesh);
	capsule->getTransform().setPosition({ -3.0f, 1.0f, 0.0f });

	// Create a cylinder
	auto cylinder = world.createGameObject<dx3d::GameObject>();
	auto cylinderMeshComp = cylinder->createOrGetComponent<dx3d::MeshComponent>();
	cylinderMeshComp->setMesh(cylinderMesh);
	cylinder->getTransform().setPosition({ 3.0f, 1.0f, 0.0f });

	// Creating a cube
	auto cube = world.createGameObject<dx3d::GameObject>();
	auto cube_meshComponent = cube->createOrGetComponent<dx3d::MeshComponent>();
	cube_meshComponent->setMesh(cubeMesh);
	cube->getTransform().setScale({ 1.0f, 1.0f, 1.0f });
	cube->getTransform().setPosition({ 0.0f, 0.0f, 0.0f });

	// Create a sphere
	auto sphere = world.createGameObject<dx3d::GameObject>();
	auto sphere_meshComponent = sphere->createOrGetComponent<dx3d::MeshComponent>();
	sphere_meshComponent->setMesh(sphereMesh);
	sphere->getTransform().setScale({ 1.0f, 1.0f, 1.0f });
	sphere->getTransform().setPosition({ -1.0f, 1.0f, 0.0f });

	// Create a circle object
	auto circle = world.createGameObject<dx3d::GameObject>();
	auto circleMeshComp = circle->createOrGetComponent<dx3d::MeshComponent>();
	circleMeshComp->setMesh(circleMesh);
	circle->getTransform().setPosition({ 1.0f, 1.0f, 0.0f });
	circle->getTransform().setRotation({ 1.57f, 0.0f, 0.0f}); // rotate 90 degrees on X-axis

	auto player = world.createGameObject<Player>();
	player->getTransform().setPosition({ 0, 1, -2 });

	getInputSystem().setCursorLocked(false);
	getInputSystem().setCursorVisible(true);
}

void MainGame::onUpdate(dx3d::f32 deltaTime)
{
	Game::onUpdate(deltaTime);
}