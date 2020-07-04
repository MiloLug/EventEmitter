#pragma once
#include <functional>
#include <string>

namespace Event {
	typedef unsigned long long elemIdNum_t;
	class HashId {
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
}