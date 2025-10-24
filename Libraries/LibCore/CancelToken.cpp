#include "CancelToken.h"

namespace LibCore
{
	namespace Async
	{
		void CancelToken::Cancel()  
		{ 
			cancelled.store(true, std::memory_order_relaxed); 
		}

		bool CancelToken::IsCancelled() const  
		{ 
			return cancelled.load(std::memory_order_relaxed); 
		}
	}
}