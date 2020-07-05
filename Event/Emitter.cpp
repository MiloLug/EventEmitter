#include "Emitter.h"
namespace Event {
	Emitter* Emitter::on(HashId id, const defaultListenerFn_t& fn, defaultListenerFn_t*& listenerPtr) {
		this->m.lock();
		std::vector<defaultListenerFn_t*>* tmp = &this->listeners[id.data];
		tmp->push_back(new defaultListenerFn_t(fn));
		listenerPtr = tmp->operator[](tmp->size() - 1);
		this->m.unlock();
		return this;
	}
	Emitter* Emitter::on(HashId id, const defaultListenerFn_t& fn) {
		this->m.lock();
		this->listeners[id.data].push_back(new defaultListenerFn_t(fn));
		this->m.unlock();
		return this;
	}

	Emitter* Emitter::off(HashId id, const defaultListenerFn_t& fn) {
		this->m.lock();
		std::vector<defaultListenerFn_t*>* tmp = &this->listeners[id.data];
		if (!tmp->empty()) {
			std::vector<defaultListenerFn_t*>::iterator cur = tmp->begin(),
				end = tmp->end();
			bool samePointers = false;
			while (cur != end) {
				if (compareFn(fn, **cur)) {
					samePointers = *cur == &fn;
					delete (*cur);
					cur = tmp->erase(cur);
					end = tmp->end();
					if (samePointers)
						break;
				}
				else {
					++cur;
				}
			}
			if (tmp->empty()) {
				this->listeners.erase(id.data);
			}
		}
		this->m.unlock();
		return this;
	}

	Emitter* Emitter::emit(HashId id, DataPack* data) {
		this->m.lock();
		std::vector<defaultListenerFn_t*>* tmp = &this->listeners[id.data];
		if (!tmp->empty()) {
			ThreadLoops::addExecutable(data, *tmp);
		}
		else {
			delete data;
		}
		this->m.unlock();
		return this;
	}

	Emitter root;
}