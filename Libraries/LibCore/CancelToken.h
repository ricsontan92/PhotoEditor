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
		class CancelToken {
		public:
			void Cancel();
			bool IsCancelled() const;

		private:
			std::atomic<bool> cancelled{ false };
		};
	}
}