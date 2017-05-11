#include "task.hpp"

#include <iostream>
#include <chrono>

int main() {
	// TEST copying and reevaluating tasks
	/*auto task = Taskify::Task<int>([]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		std::cout << "returning 8\n";
		return 8;
	});
	auto task2 = task.ContinueWith<double>([](int i){ 
		std::cout << "adding 2\n";
		return i + 2.0;
	});

	auto task3 = Taskify::Task<void>([]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		std::cout << "void task\n";
	});
	task3.Start();
	std::cout << "task 3 started\n";
	std::cout << task2.GetResult();
	std::cout << task.GetResult();
	std::cout << task.GetResult() << std::endl;
	task3.GetResult();*/

	// create vector for global functions
	std::vector<Taskify::Task<double>> tasks;
	tasks.push_back(Taskify::Task<double>([]() {
		double rtc = 0;
		for (size_t i = 0; i < 10000000; i++)
		{
			double root = std::sqrt(i);
			rtc *= 10;
			rtc += root;
			rtc /= (double)10;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		std::cout << "slow task finished\n";
		return rtc;
	}));
	tasks.push_back(Taskify::Task<double>([]() {
		std::cout << "fast task finished\n";
		return (double)10;
	}));

	// TEST WaitAny
	/*size_t i = Taskify::WaitAny(tasks);
	std::cout << tasks[i].GetResult() << std::endl;*/

	// TEST WhenAny
	/*auto t = Taskify::WhenAny(tasks);
	std::cout << (t.HasStarted() ? "New running" : "New not running") << std::endl;
	for (size_t i = 0; i < tasks.size(); i++)
	{
		std::cout << i << (tasks[i].HasStarted() ? " running" : " not running") << std::endl;
	}
	std::cout << t.GetResult() << std::endl;*/

	// TEST WaitAll
	/*Taskify::WaitAll(tasks);
	tasks[0].WaitTask();
	std::cout << tasks[0].GetResult() << " " << tasks[1].GetResult() << std::endl;*/

	// TEST WhenAll
	auto t = Taskify::WhenAll(tasks);
	std::cout << (t.HasStarted() ? "New running" : "New not running") << std::endl;
	auto result = t.GetResult();
	for (size_t i = 0; i < result.size(); i++)
	{
		std::cout << i << ":" << result[i] << std::endl;
	}

	// TEST StartNew
	auto task4 = Taskify::StartNew<void>([]() {std::cout << "Start new function test\n"; return 3; });	
}