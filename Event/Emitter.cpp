#include "Emitter.h"
namespace Event {
	Emitter* Emitter::on(HashId id, const defaultListenerFn_t& fn, defaultListenerFn_t*& listenerPtr, bool once) {
		this->m.lock();
		std::vector<ListenerContainer*>* tmp = &this->listeners[id.data];
		ListenerContainer* tmpcontainer = new ListenerContainer({ fn, once });
		tmp->push_back(tmpcontainer);
		listenerPtr = &tmpcontainer->fn;
		this->m.unlock();
		return this;
	}
	Emitter* Emitter::on(HashId id, const defaultListenerFn_t& fn, bool once) {
		this->m.lock();
		this->listeners[id.data].push_back(new ListenerContainer({ fn, once }));
		this->m.unlock();
		return this;
	}

	Emitter* Emitter::off(HashId id, const defaultListenerFn_t& fn) {
		this->m.lock();
		std::vector<ListenerContainer*>* tmp = &this->listeners[id.data];
		if (!tmp->empty()) {
			std::vector<ListenerContainer*>::iterator cur = tmp->begin(),
				end = tmp->end();
			bool samePointers = false;
			while (cur != end) {
				if (compareFn(fn, (*cur)->fn)) {
					samePointers = &(*cur)->fn == &fn;
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
		std::vector<ListenerContainer*>* tmp = &this->listeners[id.data];
		std::stack<defaultListenerFn_t> fns;

		if (!tmp->empty()) {
			std::vector<ListenerContainer*>::iterator cur = tmp->begin(),
				end = tmp->end();
			while (cur != end) {
				fns.push((*cur)->fn);
				if ((*cur)->once) {
					delete (*cur);
					cur = tmp->erase(cur);
					end = tmp->end();
				}
				else {
					++cur;
				}
			}
			if (tmp->empty()) {
				this->listeners.erase(id.data);
			}

			ThreadLoops::addExecutable(data, fns);
		}
		else {
			delete data;
		}
		this->m.unlock();
		return this;
	}

	Emitter::~Emitter() {
		this->m.lock();
		mainListenersMap_t::iterator curName = this->listeners.begin(),
			nameEnd = this->listeners.end();
		for (; curName != nameEnd; curName++) {
			std::vector<ListenerContainer*>* tmp = &curName->second;
			if (!tmp->empty()) {
				std::vector<ListenerContainer*>::iterator cur = tmp->begin(),
					end = tmp->end();
				for (; cur != end; cur++)
					delete (*cur);
			}
		}
		this->m.unlock();
	}

	Emitter root;
}