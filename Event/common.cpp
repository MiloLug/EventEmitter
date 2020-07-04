#include "common.h"

#include <functional>
#include <xxhash.h>
#include <cstring>
#include <string>

namespace Event {

	//class HashId
	inline elemIdNum_t HashId::hashString(std::string& data) {
		return (elemIdNum_t)XXH64(data.data(), data.length(), 0);
	}
	inline elemIdNum_t HashId::hashString(const char* data) {
		return (elemIdNum_t)XXH64(data, strlen(data), 0);
	}

	HashId::HashId(elemIdNum_t data) {
		this->data = data;
	}
	HashId::HashId(std::string data) {
		this->data = HashId::hashString(data);
		this->src = data;
	}
	HashId::HashId(const char* data) {
		this->data = HashId::hashString(data);
		this->src = data;
	}
	HashId::HashId(HashId& t) {
		this->data = t.data;
		this->src = t.src;
	}
	HashId::HashId(const HashId& t) {
		this->data = t.data;
		this->src = t.src;
	}
	HashId::HashId() {}

	bool HashId::operator == (std::string& a) {
		return HashId::hashString(a) == this->data;
	}
	bool HashId::operator == (const char* a) {
		return HashId::hashString(a) == this->data;
	}
	bool HashId::operator == (elemIdNum_t& a) {
		return a == this->data;
	}
	bool HashId::operator == (HashId& a) {
		return a.data == this->data;
	}

	bool HashId::operator != (std::string& a) {
		return HashId::hashString(a) != this->data;
	}
	bool HashId::operator != (const char* a) {
		return HashId::hashString(a) != this->data;
	}
	bool HashId::operator != (elemIdNum_t& a) {
		return a != this->data;
	}
	bool HashId::operator != (HashId& a) {
		return a.data != this->data;
	}
	bool HashId::operator != (const HashId& a) {
		return a.data != this->data;
	}

	std::string& HashId::operator = (std::string& a) {
		this->data = HashId::hashString(a);
		this->src = a;
		return a;
	}
	const char* HashId::operator = (const char* a) {
		this->data = HashId::hashString(a);
		this->src = a;
		return a;
	}
	elemIdNum_t HashId::operator = (elemIdNum_t a) {
		this->data = a;
		this->src = "";
		return a;
	}
	HashId& HashId::operator = (HashId& a) {
		this->data = a.data;
		this->src = a.src;
		return a;
	}
	const HashId& HashId::operator = (const HashId& a) {
		this->data = a.data;
		this->src = a.src;
		return a;
	}
}