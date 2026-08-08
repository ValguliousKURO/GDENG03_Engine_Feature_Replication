#include <DX3D/Core/Logger.h>

//events 

//#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
//#include <DX3D/EventBroadcasting/EventNames.h>
//#include <DX3D/EventBroadcasting/Parameters.h>
#include <iostream>

#include <DX3D/Core/DebugLogManager.h>

dx3d::Logger::Logger(LogLevel logLevel): m_logLevel(logLevel)
{

}

dx3d::Logger::~Logger()
{
}

void dx3d::Logger::_log(LogLevel level, const char* message)
{
	auto logLevelToString = [](LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Info: return "Info";
		case LogLevel::Warning: return "Warning";
		case LogLevel::Error: return "Error";
		default: return "Unknown";
		}
	};

	if (level > m_logLevel) return;
	std::clog << "{Dx3D " << logLevelToString(level) << "]: " << message << "\n";
	
	//event broadcast the string
	DebugLogManager::getInstance().customLog(message);

}
