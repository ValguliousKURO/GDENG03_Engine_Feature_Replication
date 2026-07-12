#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

using Callback = std::function<void()>;
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

        void addObserver(const std::string eventID, Callback cb);
        void RemoveObserver(const std::string eventID);

        // Broadcast event to all listeners
        void postEvent(const std::string eventID);

    private:
        std::unordered_map<std::string, std::vector<Callback>> listeners;
        inline static EventBroadcastManager* instancePtra = nullptr;
	};
}
