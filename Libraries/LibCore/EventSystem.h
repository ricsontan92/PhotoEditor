#pragma once
#include <functional>
#include <unordered_map>
#include <concurrent_queue.h>
#include <vector>
#include <typeindex>
#include <memory>
#include <mutex>

namespace LibCore
{
	namespace Event
	{
		struct Event 
		{
			virtual ~Event() = default;
		};

        class EventSystem {
        public:
            using EventCallback = std::function<void(const Event&)>;

            EventSystem() = default;
            ~EventSystem() = default;

            // Register listener for a specific event type
            template<typename T>
            void AddListener(std::function<void(const T&)> cb) 
            {
                std::lock_guard<std::mutex> lock{ mutex };
                auto& vec = listeners[typeid(T)];

                // wrap type-safe callback into base type
                vec.push_back([cb = std::move(cb)](const Event& e) {
                    cb(static_cast<const T&>(e));
                });
            }

            // Emit an event of specific type
            template<typename T>
            void Emit(const T& event)  
            {
                std::lock_guard<std::mutex> lock{ mutex };
                queuedEvents.push(std::make_shared<T>(event));
            }

            // Remove all listeners of a given event type
            template<typename T>
            void RemoveListeners() 
            {
                std::lock_guard<std::mutex> lock{ mutex };
                listeners.erase(typeid(T));
            }

            void RemoveAllListeners()
            {
                listeners.clear(); 
            }

            void ProcessEvents(float maxSeconds)
            {
                auto now = std::chrono::high_resolution_clock::now();

                std::shared_ptr<Event> evt;
                while (queuedEvents.try_pop(evt))
                {
                    auto it = listeners.find(std::type_index(typeid(*evt)));
                    if (it != listeners.end()) 
                    {
                        for (auto& cb : it->second)
                            cb(*evt);
                    }

                    const auto diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() / 1000.0f;
                    if (maxSeconds >= 0.0f && diffTime >= maxSeconds)
                        break;
                }
            }

        private:
            mutable std::mutex mutex;
            concurrency::concurrent_queue<std::shared_ptr<Event>> queuedEvents;
            std::unordered_map<std::type_index, std::vector<EventCallback>> listeners;
        };
	}
}