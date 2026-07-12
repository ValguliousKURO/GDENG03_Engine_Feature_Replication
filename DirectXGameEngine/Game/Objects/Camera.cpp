#include "Camera.h"
#include "iostream"
#include <algorithm>

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
	auto* m_camera = getComponent<dx3d::CameraComponent>();

	if (!m_camera) return;

	//Toggle for Perspective Mode
	if (input.isKeyDown(dx3d::KeyCode::P))
	{
		if (m_camera->getProjectionMode() != dx3d::ProjectionMode::Perspective)
		{
			m_camera->setProjectionMode(dx3d::ProjectionMode::Perspective);

			getTransform().setPosition({ 0, 1, -2 });
			getTransform().setRotation({ 0.0f, 0.0f, 0.0f });

			input.setCursorLocked(false); //change it for now
			input.setCursorVisible(true); //change it for now
		}
	}

	//Toggle for Orthographic Mode/Top Down View
	else if (input.isKeyDown(dx3d::KeyCode::O))
	{
		if (m_camera->getProjectionMode() != dx3d::ProjectionMode::Orthographic)
		{
			m_camera->setProjectionMode(dx3d::ProjectionMode::Orthographic);

			// Safely move back to original distance and unlock cursor for ortho viewing
			getTransform().setPosition({ 0.0f, 10.0f, 0.0f });
			getTransform().setRotation({ 1.5708f, 0.0f, 0.0f });
			input.setCursorLocked(false);
			input.setCursorVisible(true);
		}
	}

	//Movement and rotation controls for Perspective Mode
	if (m_camera->getProjectionMode() == dx3d::ProjectionMode::Perspective)
	{
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

		//Rotation via arrow keys
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
	}

	else if (m_camera->getProjectionMode() == dx3d::ProjectionMode::Orthographic)
	{
		//Pan Contros for Orthographic Mode WASD controls
		auto pos = getTransform().getPosition();
		auto panDirection = dx3d::Vec3(0, 0, 0);
		float panSpeed = 5.0f; //Adjust speed here

		//W and S keys shift the camera along the world Z-axis Up/Down on screen
		//A and D keys shift the camera along the world X-axis Left/Right on screen
		if (input.isKeyDown(dx3d::KeyCode::W)) panDirection.z += 1.0f;
		if (input.isKeyDown(dx3d::KeyCode::S)) panDirection.z -= 1.0f;
		if (input.isKeyDown(dx3d::KeyCode::D)) panDirection.x += 1.0f;
		if (input.isKeyDown(dx3d::KeyCode::A)) panDirection.x -= 1.0f;

		if (panDirection.length() > 0.0f)
		{
			pos = pos + dx3d::Vec3::normalize(panDirection) * panSpeed * deltaTime;
			getTransform().setPosition(pos);
		}

		float zoomSpeed = 0.02f; //Adjust zoom speed here
		float currentZoom = m_camera->getOrthoZoom();

		//Increase currentZoom to zoom out, decrease currentZoom to zoom in
		if (input.isKeyDown(dx3d::KeyCode::Up)) currentZoom -= zoomSpeed * deltaTime; // Zoom In
		if (input.isKeyDown(dx3d::KeyCode::Down)) currentZoom += zoomSpeed * deltaTime; // Zoom Out

		//Define min and max zoom to prevent flipping or extreme zooming
		currentZoom = std::clamp(currentZoom, 0.005f, 0.1f);

		m_camera->setOrthoZoom(currentZoom);
	}
}