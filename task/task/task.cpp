#include "task.hpp"

#include <iostream>
#include <chrono>

int main() {
	auto task = Taskify::Task<int>([]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		return 8;
	});
	auto task2 = task.ContinueWith<double>([](int i){ 
		std::cout << "lol" << std::endl;
		return i + 2.0;
	});

	auto task3 = Taskify::Task<void>([]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		std::cout << "void task" << std::endl;
	});
	task3.Start();
	std::cout << task2.GetResult();
	std::cout << task.GetResult();
	task3.GetResult();
}