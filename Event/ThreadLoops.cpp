#include "ThreadLoops.h"
namespace Event {
	std::mutex ThreadLoops::m;
	std::stack<ThreadLoops::fnPair_t*> ThreadLoops::fnstack;
	std::stack<std::thread*> ThreadLoops::threads;
	std::vector<std::mutex*> ThreadLoops::lockers;
	bool ThreadLoops::stop = false;

	std::condition_variable ThreadLoops::cycleLocker;
	std::mutex ThreadLoops::tm;
	std::condition_variable ThreadLoops::terminator;
	int ThreadLoops::ended;


	void ThreadLoops::executor(int id) {
		DataPack* tmpData;
		defaultListenerFn_t tmpFn;
		while (!ThreadLoops::stop) {
			if (ThreadLoops::getExecutable(tmpData, tmpFn)) {
				tmpFn(tmpData);
				if (tmpData != nullptr)
					delete tmpData;
			}
			else {
				std::unique_lock<std::mutex> lck(*ThreadLoops::lockers[id]);
				ThreadLoops::cycleLocker.wait(lck, ThreadLoops::canContinue);
			}
		}
		ThreadLoops::m.lock();
		ThreadLoops::ended++;
		ThreadLoops::terminator.notify_all();
		ThreadLoops::m.unlock();
	}

	inline bool ThreadLoops::canContinue() {
		return ThreadLoops::fnstack.size() > 0 || ThreadLoops::stop;
	}

	inline bool ThreadLoops::canTerminate() {
		return ThreadLoops::ended >= ThreadLoops::threads.size();
	}

	bool ThreadLoops::getExecutable(DataPack*& data, defaultListenerFn_t& fn) {
		std::unique_lock<std::mutex> ul(ThreadLoops::m);
		if (ThreadLoops::fnstack.empty()) {
			return false;
		}
		else {
			fnPair_t* tmp = ThreadLoops::fnstack.top();

			data = tmp->first == nullptr ? nullptr : tmp->first->copy();
			fn = tmp->second.top();
			tmp->second.pop();

			if (tmp->second.empty()) {
				if (tmp->first != nullptr)
					delete tmp->first;
				delete tmp;
				ThreadLoops::fnstack.pop();
			}

			return true;
		}
	}

	void ThreadLoops::addExecutable(DataPack* data, const defaultListenerFn_t& fn) {
		std::lock_guard<std::mutex> guard(ThreadLoops::m);
		ThreadLoops::fnstack.push(new fnPair_t({
				data,
				std::stack<defaultListenerFn_t>({fn})
			}));
		ThreadLoops::cycleLocker.notify_all();
	}

	void ThreadLoops::addExecutable(DataPack* data, std::stack<defaultListenerFn_t>& fns) {
		std::lock_guard<std::mutex> guard(ThreadLoops::m);
		ThreadLoops::fnstack.push(new fnPair_t({
				data,
				fns
			}));
		ThreadLoops::cycleLocker.notify_all();
	}

	void ThreadLoops::addExecutable(DataPack* data, std::vector<defaultListenerFn_t>& fns) {
		std::lock_guard<std::mutex> guard(ThreadLoops::m);
		ThreadLoops::fnstack.push(new fnPair_t({
				data,
				std::stack<defaultListenerFn_t>(std::deque<defaultListenerFn_t>(fns.begin(), fns.end()))
			}));
		ThreadLoops::cycleLocker.notify_all();
	}

	void ThreadLoops::addExecutable(DataPack* data, std::vector<defaultListenerFn_t*>& fns) {
		std::lock_guard<std::mutex> guard(ThreadLoops::m);

		fnPair_t* tmpPair = new fnPair_t({
			data,
			{}
			});

		std::vector<defaultListenerFn_t*>::iterator cur = fns.begin(),
			end = fns.end();
		for (; cur != end; cur++) {
			tmpPair->second.push(**cur);
		}
		ThreadLoops::fnstack.push(tmpPair);
		ThreadLoops::cycleLocker.notify_all();
	}

	void ThreadLoops::init(size_t threadsNumber) {
		std::unique_lock<std::mutex> lck(ThreadLoops::m);

		ThreadLoops::ended = 0;
		for (size_t i = 0; i < threadsNumber; i++) {
			std::thread* tmp = new std::thread(ThreadLoops::executor, i);
			tmp->detach();
			ThreadLoops::threads.push(tmp);
			ThreadLoops::lockers.push_back(new std::mutex);
		}
	}
	void ThreadLoops::init() {
		size_t proc_count = std::thread::hardware_concurrency();
		ThreadLoops::init(proc_count);
	}

	void ThreadLoops::wait() {
		std::unique_lock<std::mutex> lck(ThreadLoops::tm);
		ThreadLoops::terminator.wait(lck, ThreadLoops::canTerminate);
		lck.unlock();
		std::unique_lock<std::mutex> lck2(ThreadLoops::m);
		
		size_t threadsNumber = ThreadLoops::threads.size();
		for (size_t i = 0; i < threadsNumber; i++) {
			std::thread* t = ThreadLoops::threads.top();
			delete t;
			ThreadLoops::threads.pop();
		}
		std::vector<std::mutex*>::iterator cur = ThreadLoops::lockers.begin(),
			end = ThreadLoops::lockers.end();
		for (; cur != end; cur++) {
			delete* cur;
		}
		ThreadLoops::lockers.clear();
	}

	void ThreadLoops::terminate(bool dontWaitFunctions) {
		std::lock_guard<std::mutex> guard(ThreadLoops::m);
		ThreadLoops::stop = true;
		ThreadLoops::cycleLocker.notify_all();
		if (dontWaitFunctions) {
			ThreadLoops::ended = ThreadLoops::threads.size();
			ThreadLoops::terminator.notify_all();
		}
	}
}