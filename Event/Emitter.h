#pragma once
#include <vector>
#include <map>
#include "common.h"
#include <functional>
#include "DataPack.h"
#include "ThreadLoops.h"
#include <mutex>

namespace Event {
	typedef std::map<elemIdNum_t, std::vector<defaultListenerFn_t*>> mainListenersMap_t;
	class Emitter {
	private:
		std::mutex m;
		mainListenersMap_t listeners;
	public:
		Emitter* on(HashId id, const defaultListenerFn_t& fn, defaultListenerFn_t*& listenerPtr);
		Emitter* on(HashId id, const defaultListenerFn_t& fn);

		Emitter* off(HashId id, const defaultListenerFn_t& fn);

		Emitter* emit(HashId id, DataPack* data = nullptr);
	};
	extern Emitter root;
	inline void init(size_t maxThreadsNumber = 2) {
		ThreadLoops::init(maxThreadsNumber);
	};
	inline void wait() {
		ThreadLoops::wait();
	};
	inline void terminate(bool dontWaitFunctions = false) {
		ThreadLoops::terminate(dontWaitFunctions);
	};
	inline Emitter* on(HashId id, const defaultListenerFn_t& fn, defaultListenerFn_t*& listenerPtr) {
		return root.on(id, fn, listenerPtr);
	};
	inline Emitter* on(HashId id, const defaultListenerFn_t& fn) {
		return root.on(id, fn);
	};
	inline Emitter* off(HashId id, const defaultListenerFn_t& fn) {
		return root.off(id, fn);
	};
	inline Emitter* emit(HashId id, DataPack* data = nullptr) {
		return root.emit(id, data);
	};
}