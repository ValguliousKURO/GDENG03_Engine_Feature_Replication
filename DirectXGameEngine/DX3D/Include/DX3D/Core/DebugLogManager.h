#pragma once
#include <DX3D/Core/Base.h>

namespace dx3d
{
	/// 
	/// / enum states of error? no
	/// 
	/// enum class LogLevel
	/// {
	/// Error = 0,
	///Warning,
	///Info
	/// }
 
	/// 

	class DebugLogManager 
	{
	public:
		DebugLogManager();

		DebugLogManager(const DebugLogManager&) = delete;
		DebugLogManager& operator=(const DebugLogManager&) = delete;

		static DebugLogManager& getInstance() {
			static DebugLogManager instance;
			return instance;
		}


	public:
		void customLog(const std::string& message);
		
		
		
	private:
	};

}