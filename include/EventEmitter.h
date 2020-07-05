#pragma once
#include <functional>
#include <string>
#include <thread>
#include <stack>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <map>

#ifdef DLLEVENTEMITTER_EXPORTS
	#define DLL_EVENTEMITTER_API __declspec(dllexport)
#else
	#define DLL_EVENTEMITTER_API __declspec(dllimport)
#endif

namespace Event {
	typedef unsigned long long elemIdNum_t;

	class DLL_EVENTEMITTER_API HashId {
	public:
		static inline elemIdNum_t hashString(std::string& data);
		static inline elemIdNum_t hashString(const char* data);
		elemIdNum_t data = 0;
		std::string src = "";
		HashId(elemIdNum_t data);
		HashId(std::string data);
		HashId(const char* data);
		HashId(HashId& t);
		HashId(const HashId& t);
		HashId();

		bool operator == (std::string& a);
		bool operator == (const char* a);
		bool operator == (elemIdNum_t& a);
		bool operator == (HashId& a);

		bool operator != (std::string& a);
		bool operator != (const char* a);
		bool operator != (elemIdNum_t& a);
		bool operator != (HashId& a);
		bool operator != (const HashId& a);

		std::string& operator = (std::string& a);
		const char* operator = (const char* a);
		elemIdNum_t operator = (elemIdNum_t a);
		HashId& operator = (HashId& a);
		const HashId& operator = (const HashId& a);
	};

	template<typename T, typename... U>
	inline bool fnCanBeCompared(const std::function<T(U...)>& f) {
		typedef T(fnType)(U...);
		fnType* const* fnPointer = f.template target<fnType*>();
		return fnPointer != nullptr;
	}

	template<typename T, typename... U>
	inline bool comparableCompare(const std::function<T(U...)>& f1, const std::function<T(U...)>& f2) {
		typedef T(fnType)(U...);
		fnType* const* fnPointer1 = f1.template target<fnType*>();
		fnType* const* fnPointer2 = f2.template target<fnType*>();
		return fnPointer1 != nullptr
			&& fnPointer2 != nullptr
			&& *fnPointer1 == *fnPointer2;
	}

	template<typename T, typename... U>
	bool compareFn(const std::function<T(U...)>& fnA, const std::function<T(U...)>& fnB) {
		bool fnAComparable = fnCanBeCompared(fnA);
		bool fnBComparable = fnCanBeCompared(fnB);
		if (fnAComparable ^ fnBComparable) {
			return false;
		}
		else if (fnBComparable) {
			return comparableCompare(fnA, fnB);
		}
		else {
			return &fnA == &fnB;
		}
	}


	class DataPack
	{
	public:
		void* data;
		DataPack(void* d) {
			this->data = d;
		}
		DataPack() {}
		virtual DataPack* copy() {
			DataPack* t = new DataPack();
			t->data = this->data;
			return t;
		}
		virtual ~DataPack() {}
	};

	template<typename T = void*>
	class DataPackCast : public DataPack {
	public:
		T data;
		DataPackCast(T d) {
			this->data = d;
		}
		DataPackCast() {}
		virtual DataPackCast* copy() {
			DataPackCast<T>* t = new DataPackCast<T>();
			t->data = this->data;
			return t;
		}
		virtual ~DataPackCast() {}
	};

	template<typename T>
	class DataPackAutoClean : public DataPack {
	private:
		int* copyCount = new int;
	public:
		T* data;
		DataPackAutoClean(T* d) {
			this->data = d;
			*this->copyCount = 0;
		}
		DataPackAutoClean() {
			*this->copyCount = 0;
		}
		virtual DataPackAutoClean* copy() {
			DataPackAutoClean<T>* t = new DataPackAutoClean<T>();
			delete t->copyCount;

			++(*this->copyCount);
			t->copyCount = this->copyCount;
			t->data = this->data;
			return t;
		}
		virtual ~DataPackAutoClean() {
			if (*this->copyCount == 0) {
				delete this->copyCount;
				delete this->data;
			}
			else {
				--(*this->copyCount);
			}
		}
	};

	typedef std::function<void(DataPack*)> defaultListenerFn_t;

	class DLL_EVENTEMITTER_API ThreadLoops
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

	typedef std::map<elemIdNum_t, std::vector<defaultListenerFn_t*>> mainListenersMap_t;

	class DLL_EVENTEMITTER_API Emitter {
	private:
		std::mutex m;
		mainListenersMap_t listeners;
	public:
		Emitter* on(HashId id, const defaultListenerFn_t& fn, defaultListenerFn_t*& listenerPtr);
		Emitter* on(HashId id, const defaultListenerFn_t& fn);

		Emitter* off(HashId id, const defaultListenerFn_t& fn);

		Emitter* emit(HashId id, DataPack* data = nullptr);
	};
	extern DLL_EVENTEMITTER_API Emitter root;
	inline DLL_EVENTEMITTER_API void init(size_t maxThreadsNumber = 2) {
		ThreadLoops::init(maxThreadsNumber);
	};
	inline DLL_EVENTEMITTER_API void wait() {
		ThreadLoops::wait();
	};
	inline DLL_EVENTEMITTER_API void terminate(bool dontWaitFunctions = false) {
		ThreadLoops::terminate(dontWaitFunctions);
	};
	inline DLL_EVENTEMITTER_API Emitter* on(HashId id, const defaultListenerFn_t& fn, defaultListenerFn_t*& listenerPtr) {
		return root.on(id, fn, listenerPtr);
	};
	inline DLL_EVENTEMITTER_API Emitter* on(HashId id, const defaultListenerFn_t& fn) {
		return root.on(id, fn);
	};
	inline DLL_EVENTEMITTER_API Emitter* off(HashId id, const defaultListenerFn_t& fn) {
		return root.off(id, fn);
	};
	inline DLL_EVENTEMITTER_API Emitter* emit(HashId id, DataPack* data = nullptr) {
		return root.emit(id, data);
	};
}