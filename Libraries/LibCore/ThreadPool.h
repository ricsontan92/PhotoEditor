#pragma once

#include <condition_variable>
#include <functional>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <atomic>

#include "Future.h"

namespace LibCore
{
	namespace Async 
	{
		class Executor {
		public:
			virtual ~Executor() = default;
			virtual void Submit(std::function<void()> task) = 0;
		};

        class ThreadPool : public Executor 
        {
        public:
            explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency())
                : stopFlag(false)
            {
                for (size_t i = 0; i < threadCount; ++i) 
                {
                    workers.emplace_back([this] {
                        WorkerLoop();
                    });
                }
            }

            ~ThreadPool() override 
            {
                Shutdown();
            }

            void Submit(std::function<void()> task) override 
            {
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    tasks.push(std::move(task));
                }
                queueCond.notify_one();
            }

            template<typename F, typename... Args>
            auto Enqueue(F&& f, Args&&... args)-> Future<std::invoke_result_t<F, Args...>>
            {
                using ReturnType = std::invoke_result_t<F, Args...>;
                auto promise = std::make_shared<std::promise<ReturnType>>();
                auto future = Future<ReturnType>(std::move(promise->get_future()));

                auto boundTask = [promise, func = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable {
                    try {
                        if constexpr (std::is_void_v<ReturnType>) {
                            func(std::forward<Args>(args)...);
                            promise->set_value();
                        }
                        else {
                            promise->set_value(func(std::forward<Args>(args)...));
                        }
                    }
                    catch (...) {
                        promise->set_exception(std::current_exception());
                    }
                };

                Submit(std::move(boundTask));
                return future;
            }

            void Shutdown() {
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    stopFlag = true;
                }
                queueCond.notify_all();

                for (auto& t : workers) 
                {
                    if (t.joinable()) 
                        t.join();
                }
            }

        private:
            std::vector<std::thread> workers;
            std::queue<std::function<void()>> tasks;
            std::mutex queueMutex;
            std::condition_variable queueCond;
            std::atomic<bool> stopFlag;

            void WorkerLoop() 
            {
                while (true) 
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock{ queueMutex };
                        queueCond.wait(lock, [&] { return stopFlag || !tasks.empty(); });
                        if (stopFlag && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            }
        };
	}
}