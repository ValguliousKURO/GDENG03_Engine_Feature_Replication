#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <iostream>

dx3d::EventBroadcastManager::EventBroadcastManager()
{

}

void dx3d::EventBroadcastManager::RemoveObserver(const std::string eventID)
{
	if (listenersNoParams.contains(eventID)) listenersNoParams.erase(eventID);
	if (listenersWithParams.contains(eventID)) listenersWithParams.erase(eventID);
}


// --- Add observer without parameters ---
void dx3d::EventBroadcastManager::EventBroadcastManager::addObserver(const std::string& eventID, CallbackNoParams cb) {
	listenersNoParams[eventID].push_back(cb);
}

// --- Add observer with parameters ---
void dx3d::EventBroadcastManager::EventBroadcastManager::addObserver(const std::string& eventID, CallbackWithParams cb) {
	listenersWithParams[eventID].push_back(cb);
}

// --- Post event without parameters ---
void dx3d::EventBroadcastManager::EventBroadcastManager::postEvent(const std::string& eventID) {
    auto it = listenersNoParams.find(eventID);
    if (it != listenersNoParams.end()) {
        auto callbacks = it->second;
        for (auto& cb : callbacks) {
            cb(); // invoke no-param callback
        }
    }
    else {
        std::cout << "No no-param listeners for event \"" << eventID << "\"\n";
    }
}

// --- Post event with parameters ---
void dx3d::EventBroadcastManager::EventBroadcastManager::postEvent(const std::string& eventID, Parameters& params) {
    auto it = listenersWithParams.find(eventID);
    if (it != listenersWithParams.end()) {
        auto callbacks = it->second;
        for (auto& cb : callbacks) {
            cb(params); // invoke with-param callback
        }
    }
    else {
        std::cout << "No param-based listeners for event \"" << eventID << "\"\n";
    }
}
