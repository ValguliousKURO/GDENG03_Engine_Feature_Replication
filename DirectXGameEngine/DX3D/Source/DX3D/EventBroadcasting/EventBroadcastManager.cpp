#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <iostream>

dx3d::EventBroadcastManager::EventBroadcastManager()
{

}

void dx3d::EventBroadcastManager::addObserver(const std::string eventID, Callback cb)
{
    listeners[eventID].push_back(cb);
}

void dx3d::EventBroadcastManager::RemoveObserver(const std::string eventID)
{
    listeners.erase(eventID);
}

void dx3d::EventBroadcastManager::postEvent(const std::string eventID)
{
    auto it = listeners.find(eventID);
    if (it != listeners.end()) {
        for (auto& cb : it->second) {
            cb(); // invoke callback
        }
    }
    else {
        std::cout << "No listeners for event \"" << eventID << "\"\n";
    }
}
