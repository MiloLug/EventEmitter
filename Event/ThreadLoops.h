#pragma once
#include <thread>
#include <stack>
#include <functional>
#include "DataPack.h"
#include <mutex>
#include <vector>
#include <condition_variable>

namespace Event {
	typedef std::function<void(DataPack*)> defaultListenerFn_t;

	class ThreadLoops
	{
	private:
		typedef std::pair<DataPack*, std::stack<defaultListenerFn_t>> fnPair_t;

		static std::mutex m;
		static std::stack<fnPair_t*> fnstack;
		static std::stack<std::thread*> threads;
		static std::vector<std::mutex*> lockers;

		static std::condition_variable cycleLocker;
		static std::mutex tm;
		static std::condition_variable terminator;
		static int ended;

		static void executor(int id);

		static inline bool canContinue();
		static inline bool canTerminate();

		static bool getExecutable(DataPack*& data, defaultListenerFn_t& fn);

	public:
		static bool stop;

		static void addExecutable(DataPack* data, const defaultListenerFn_t& fn);

		static void addExecutable(DataPack* data, std::stack<defaultListenerFn_t>& fns);

		static void addExecutable(DataPack* data, std::vector<defaultListenerFn_t>& fns);

		static void addExecutable(DataPack* data, std::vector<defaultListenerFn_t*>& fns);

		static void init(size_t threadsNumber);
		static void init();

		static void wait();

		static void terminate(bool dontWaitFunctions = false);
	};
}

