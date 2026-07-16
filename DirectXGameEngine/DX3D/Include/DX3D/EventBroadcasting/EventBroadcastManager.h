#pragma once
#include <DX3D/EventBroadcasting/Parameters.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

using CallbackNoParams = std::function<void()>;
using CallbackWithParams = std::function<void(dx3d::Parameters&)>;
namespace dx3d
{
	class EventBroadcastManager
	{
    public:
        EventBroadcastManager();

        EventBroadcastManager(const EventBroadcastManager&) = delete;
        EventBroadcastManager& operator=(const EventBroadcastManager&) = delete;

        static EventBroadcastManager& getInstance() {
            static EventBroadcastManager instance;
            return instance;
        }

        void RemoveObserver(const std::string eventID);

        void addObserver(const std::string& eventID, CallbackNoParams cb);
        void addObserver(const std::string& eventID, CallbackWithParams cb);

        void postEvent(const std::string& eventID);
        void postEvent(const std::string& eventID, Parameters& params);

    private:
        std::unordered_map<std::string, std::vector<CallbackNoParams>> listenersNoParams;
        std::unordered_map<std::string, std::vector<CallbackWithParams>> listenersWithParams;
        inline static EventBroadcastManager* instancePtra = nullptr;
	};
}
