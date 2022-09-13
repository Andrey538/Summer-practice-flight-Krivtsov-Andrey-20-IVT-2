#pragma once
#include <chrono>
#include "Types.h"

namespace Andrew
{
	class Timer
	{
	public:
		Timer()
		{
			time = std::chrono::steady_clock::now();
		}
		double getElapsed()
		{
			auto now = std::chrono::steady_clock::now();
			double res = (now - time).count();
			time = now;
			return res;
		}
	private:
		std::chrono::steady_clock::time_point time;
	};
}