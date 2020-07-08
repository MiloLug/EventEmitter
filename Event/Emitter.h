#pragma once
#include <vector>
#include <map>
#include "common.h"
#include <functional>
#include "DataPack.h"
#include "ThreadLoops.h"
#include <mutex>

namespace Event {
	struct ListenerContainer {
		defaultListenerFn_t fn;
		bool once = false;
	};

	typedef std::map<elemIdNum_t, std::vector<ListenerContainer*>> mainListenersMap_t;

	class Emitter {
	private:
		std::mutex m;
		mainListenersMap_t listeners;
	public:
		Emitter* on(HashId id, const defaultListenerFn_t& fn, defaultListenerFn_t*& listenerPtr, bool once = false);
		Emitter* on(HashId id, const defaultListenerFn_t& fn, bool once = false);

		inline Emitter* once(HashId id, const defaultListenerFn_t& fn, defaultListenerFn_t*& listenerPtr) {
			return this->on(id, fn, listenerPtr, true);
		};
		inline Emitter* once(HashId id, const defaultListenerFn_t& fn) {
			return this->on(id, fn, true);
		};

		Emitter* off(HashId id, const defaultListenerFn_t& fn);

		Emitter* emit(HashId id, DataPack* data = nullptr);
		virtual ~Emitter();
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
	inline Emitter* once(HashId id, const defaultListenerFn_t& fn, defaultListenerFn_t*& listenerPtr) {
		return root.on(id, fn, listenerPtr, true);
	};
	inline Emitter* once(HashId id, const defaultListenerFn_t& fn) {
		return root.on(id, fn, true);
	};
	inline Emitter* off(HashId id, const defaultListenerFn_t& fn) {
		return root.off(id, fn);
	};
	inline Emitter* emit(HashId id, DataPack* data = nullptr) {
		return root.emit(id, data);
	};
}