#include <iostream>
#include <chrono>
#include <string>
#include "Event/Emitter.h"
#include <mutex>

using namespace std;

int main() {
	Event::init();


	struct TmpStruct {
		int count;
		std::string id;
	};

	std::mutex m;

	Event::on("exit", [&m](Event::DataPack*) {
		m.lock();
		std::cout << "exit!" << std::endl;
		m.unlock();
		Event::terminate(true);
	});

	Event::on("tester", [&m](Event::DataPack* dt) {
		TmpStruct* tmp = &((Event::DataPackCast<TmpStruct>*)dt)->data;

		for (int i = 0; i < tmp->count; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			m.lock();
			std::cout << "test: " << tmp->id << " _ " << i << std::endl;
			m.unlock();
		}
	});

	Event::emit("tester", new Event::DataPackCast<TmpStruct>({
		10,
		"first"
		}));

	std::this_thread::sleep_for(std::chrono::milliseconds(6000));

	std::cout << "main thread!" << std::endl;

	Event::emit("tester", new Event::DataPackCast<TmpStruct>({
		6,
		"second"
		}));

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

	Event::emit("exit");

	Event::emit("tester", new Event::DataPackCast<TmpStruct>({
		6,
		"third"
		}));

	Event::wait();

	return 0;
}