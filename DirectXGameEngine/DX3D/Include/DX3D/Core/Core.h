#pragma once
#include <stdexcept>
#include <memory>

#define dx3d_disable_copy_and_move(Class)	\
protected:\
	Class(const Class&) = delete;        \
	Class& operator=(const Class&) = delete; \
	Class(Class&&) = delete;             \
	Class& operator=(Class&&) = delete;

namespace dx3d
{
	class Base;
	class Window;
	class Game;
	class InputSystem;

	class GraphicsEngine;
	class GraphicsDevice;

	class Logger; 
	class SwapChain;
	class Display;

	class DeviceContext;

	class ShaderBinary;
	class GraphicsPipelineState;

	class VertexBuffer;
	class VertexShaderSignature;

	class ConstantBuffer;

	class IndexBuffer;

	class Texture;
	class Sampler;
	class GraphicsPipelineLayout;

	class World;
	class GameObject;

	class Component;
	class TransformComponent;

	class WorldRenderer;

	class ResourceManager;
	class Resource;
	class MaterialResource;
	class TextureResource;

	// new stuff here

	class TestUI;
	class Rasterizer;

	using i32 = int;
	using ui32 = unsigned int;
	using f32 = float;
	using d64 = double;

	template <typename T> using RefPtr = std::shared_ptr<T>;
	template <typename T> using UniquePtr = std::unique_ptr<T>;
}