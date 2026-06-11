#include "MainGame.h"
#include "Objects/Player.h"


MainGame::MainGame(const dx3d::GameDesc& desc) : dx3d::Game(desc)
{
}

void MainGame::onCreate()
{
	Game::onCreate();
	auto& world = getWorld();

	auto floor = world.createGameObject<dx3d::GameObject>();
	floor->createOrGetComponent<dx3d::CubeComponent>();
	floor->getTransform().setScale({ 6.8f, 0.1f, 6.8f });
	floor->getTransform().setPosition({ 0, -1, 0 });


	srand((unsigned int)time(NULL));

	for (auto y = -2; y < 3; y++)
	{
		for (auto x = -2; x < 3; x++)
		{
			auto cube = world.createGameObject<dx3d::GameObject>();
			cube->createOrGetComponent<dx3d::CubeComponent>();
			auto height = (rand() % 120) + (80.0f);
			height /= 100.0f;

			auto width = (rand() % 600) + (200.0f);
			width /= 1000.0f;

			cube->getTransform().setScale({ width, height, width });
			cube->getTransform().setPosition({ x * 1.4f, (height / 2.0f) - 1.0f, y * 1.4f });
		}
	}

	auto player = world.createGameObject<Player>();
	player->getTransform().setPosition({ 0, 1, -2 });

	getInputSystem().setCursorLocked(false);
	getInputSystem().setCursorVisible(true);
}

void MainGame::onUpdate(dx3d::f32 deltaTime)
{
	Game::onUpdate(deltaTime);
}