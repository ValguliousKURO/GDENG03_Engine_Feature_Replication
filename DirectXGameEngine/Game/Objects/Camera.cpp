#include "Camera.h"
#include "iostream"

Camera::Camera(const dx3d::GameObjectDesc& desc) : dx3d::GameObject(desc)
{
}

Camera::~Camera()
{
}

void Camera::onCreate()
{
	createOrGetComponent<dx3d::CameraComponent>();
}

void Camera::onUpdate(dx3d::f32 deltaTime)
{
	auto& input = getInputSystem();

	float rot_speed = 2.5f;
	auto rot = getTransform().getRotation();

	/*auto sensitivity = 0.001f;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::MouseLeft))
	{
		rot.x += getInputSystem().getMouseDelta().y * sensitivity;
		rot.y += getInputSystem().getMouseDelta().x * sensitivity;
		input.setCursorLocked(true);
		input.setCursorVisible(false);
	}
	else
	{
		input.setCursorLocked(false);
		input.setCursorVisible(true);
	}*/

	// Rotation via arrow keys
	if (getInputSystem().isKeyDown(dx3d::KeyCode::Up)) rot.x -= rot_speed * deltaTime;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::Down)) rot.x += rot_speed * deltaTime;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::Left)) rot.y -= rot_speed * deltaTime;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::Right)) rot.y += rot_speed * deltaTime;

	if (rot.x > 1.57f) rot.x = 1.57f;
	else if (rot.x < -1.57f) rot.x = -1.57f;
	getTransform().setRotation(rot);

	auto pos = getTransform().getPosition();
	auto forward = 0.0f;
	auto right = 0.0f;
	auto speed = 3.0f;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::W)) forward = 1.0f;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::S)) forward = -1.0f;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::D)) right = 1.0f;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::A)) right = -1.0f;
	auto forwardDir = getTransform().forward() * forward;
	auto rightDir = getTransform().right() * right;
	auto direction = dx3d::Vec3::normalize(forwardDir + rightDir);
	pos = pos + direction * speed * deltaTime;
	getTransform().setPosition(pos);

	if (getInputSystem().isKeyPressed(dx3d::KeyCode::E)) std::cout << "Hallo ";
}