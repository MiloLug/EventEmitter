#pragma once

namespace Event {

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
}

