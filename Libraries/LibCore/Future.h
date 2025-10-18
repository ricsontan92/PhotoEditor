#pragma once

#include <future>

namespace LibCore
{
	namespace Async
	{
		template<typename T>
		class Future
		{
		public:
			explicit Future() : future{} {}
			explicit Future(std::future<T>&& fut) : future{ std::move(fut) } {}
			bool Valid() const { return future.valid(); }
			T Get() { return future.get(); }
			void Wait() { future.wait(); }
			void WaitFor(const unsigned int ms) { future.wait_for(std::chrono::milliseconds{ ms }); }
			bool IsReady() const { return Valid() && future.wait_for(std::chrono::milliseconds{ 0 }) == std::future_status::ready; }

		private:
			std::future<T> future;
		};

		template<typename F, typename... Args>
		auto Run(F&& f, Args&&... args) {
			using ret_t = std::invoke_result_t<F, Args...>;
			// launch the job on std::async and return Future<ret_t>
			auto stdf = std::async(std::launch::async, [f = std::forward<F>(f), args_tuple = std::make_tuple(std::forward<Args>(args)...)]() mutable -> ret_t {
				// unpack tuple and call
				return std::apply(f, std::move(args_tuple));
			});
			return Future<ret_t>(std::move(stdf));
		}
	}
}