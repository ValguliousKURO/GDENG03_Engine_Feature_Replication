#include <DX3D/Core/DebugLogManager.h>

#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>
#include <DX3D/EventBroadcasting/Parameters.h>
#include <iostream>

dx3d::DebugLogManager::DebugLogManager()
{
}

void dx3d::DebugLogManager::customLog(const std::string& message)
{
	auto params = dx3d::Parameters{};
	params.PutExtra("Message", std::string(message));

	dx3d::EventBroadcastManager::getInstance().postEvent(dx3d::EventNames::ON_DEBUG_LOG_ENTRY, params);
}
