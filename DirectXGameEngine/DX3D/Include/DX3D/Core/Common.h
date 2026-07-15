#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Core/Logger.h>
#include <DX3D/Math/Rect.h>

namespace dx3d
{
	struct BaseDesc
	{
		Logger& logger;
	};

	struct WindowDesc
	{
		BaseDesc base;
		Rect size;
	};

	struct DisplayDesc
	{
		WindowDesc window;
		GraphicsDevice& graphicsDevice;
	};

	struct GraphicsDeviceDesc
	{
		BaseDesc base;
	};

	struct SwapChainDesc
	{
		void* winHandle{};
		Rect winSize{};
	};

	enum class ShaderType
	{
		VertexShader = 0,
		PixelShader
	};

	struct ShaderCompileDesc
	{
		const char* shaderSourceName{};
		const char* shaderSourceCode{};
		size_t shaderSourceCodeSize{};
		const char* shaderEntryPoint{};
		ShaderType shaderType{};
	};

	struct GraphicsPipelineStateDesc
	{
		const GraphicsPipelineLayout& layout;
	};

	struct VertexBufferDesc
	{
		const void* vertexList{};
		ui32 vertexListSize{};
		ui32 vertexSize{};
	};

	struct GraphicsPipelineLayoutDesc
	{
		const RefPtr<ShaderBinary>& vsBinary;
		const RefPtr<ShaderBinary>& psBinary;
	};

	struct BinaryData
	{
		const void* data{};
		size_t dataSize{};
	};

	struct ConstantBufferDesc
	{
		const void* buffer{};
		ui32 bufferSize{};
	};

	struct IndexBufferDesc
	{
		const ui32* indexList{};
		ui32 indexListSize{};
	};
	//newstuff here

	struct RasterizerDesc
	{
		bool isWire{ false }; // false = solid, true = wireframe
	};

	// Place all systems/managers here
	struct GameContext
	{
		InputSystem& input;
		ResourceManager& resourceManager;

		GraphicsDevice& device;
	};

	struct GameDesc
	{
		Rect windowSize{ 1280, 720 };
		Logger::LogLevel logLevel = Logger::LogLevel::Error;
	};

	struct ResourceDesc
	{
		BaseDesc base;
		const wchar_t* path{};
		ResourceManager& manager;
	};

	struct MaterialResourceDesc
	{
		ResourceDesc base;
		GraphicsDevice& graphicsDevice;
	};

	struct TextureResourceDesc
	{
		ResourceDesc base;
		GraphicsDevice& graphicsDevice;
	};

	struct SystemContext
	{
		GraphicsDevice& graphicsDevice;
	};

	struct ResourceManagerDesc
	{
		BaseDesc base;
		SystemContext context;
	};

	struct WorldDesc
	{
		BaseDesc base;
		GameContext gameContext;
	};

	struct GameObjectDesc
	{
		BaseDesc base;
		GameContext gameContext;
		World& world;

		// New: optional per‑window input system
		InputSystem* windowInput{};
	};

	struct ComponentDesc
	{
		BaseDesc base;
		GameObject& object;
		World& world;
		GameContext& context;
	};

	struct WorldRendererDesc
	{
		BaseDesc base;
		GraphicsDevice& engine;
	};

	struct TextureDesc
	{
		Rect size{};
		const void* pixels{};
	};
	struct SamplerDesc
	{
	};

	enum class KeyCode
	{
		Unknown = 0,
		// Letters
		A, B, C, D, E, F, G,
		H, I, J, K, L, M, N,
		O, P, Q, R, S, T, U,
		V, W, X, Y, Z,

		// Numbers
		Num0,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,


		Escape,
		Shift,
		Space,
		Enter,

		// Mouse buttons (optional inclusion)
		MouseLeft,
		MouseRight,
		MouseMiddle,

		// Arrows
		Up,
		Down,
		Left,
		Right,

		Count
	};

	struct InputSystemDesc
	{
		BaseDesc base;
	};

}