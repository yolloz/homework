// MICHAL JURCO

#include "task.hpp"

#include <iostream>
#include <chrono>

/*
	Main inlcudes tests for all member and global functions
	First part contains tests for templated task with int and double tasks
	Second part contains tests for void task
	
	To run tests, just uncommnet desired test
	Global functions require vector section uncommented
*/

int main() {
	// TEST copying and reevaluating tasks
	/*std::cout << std::endl << "TESTING templated copying and reevaluation\n"<< std::endl;
	auto task = Taskify::Task<int>([]() {
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
	/*std::vector<Taskify::Task<double>> tasks;
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
	}));*/
	
	// TEST WaitAny
	/*std::cout << std::endl << "TESTING templated WaitAny\n"<< std::endl;
	size_t i = Taskify::WaitAny(tasks);
	std::cout << tasks[i].GetResult() << std::endl;*/

	// TEST WhenAny
	/*std::cout << std::endl << "TESTING templated WhenAny\n"<< std::endl;
	auto t = Taskify::WhenAny(tasks);
	std::cout << (t.HasStarted() ? "New running" : "New not running") << std::endl;
	for (size_t i = 0; i < tasks.size(); i++)
	{
		std::cout << i << (tasks[i].HasStarted() ? " running" : " not running") << std::endl;
	}
	std::cout << t.GetResult() << std::endl;*/

	// TEST WaitAll
	/*std::cout << std::endl << "TESTING templated WaitAll\n"<< std::endl;
	Taskify::WaitAll(tasks);
	tasks[0].WaitTask();
	std::cout << tasks[0].GetResult() << " " << tasks[1].GetResult() << std::endl;*/

	// TEST WhenAll
	/*std::cout << std::endl << "TESTING templated WHenAll\n"<< std::endl;
	auto t = Taskify::WhenAll(tasks);
	std::cout << (t.HasStarted() ? "New running" : "New not running") << std::endl;
	auto result = t.GetResult();
	for (size_t i = 0; i < result.size(); i++)
	{
		std::cout << i << ":" << result[i] << std::endl;
	}*/

	// TEST StartNew
	/*std::cout << std::endl << "TESTING StartNew\n" << std::endl;
	auto task4 = Taskify::StartNew<void>([]() {std::cout << "Start new function test\n"; return 3; });*/	



	///*** VOID TEST ***///

	// TEST copying and reevaluating tasks
	std::cout << std::endl << "TESTING void copying and reevaluating\n" << std::endl;
	auto task1 = Taskify::Task<void>([]() {
	std::cout << "task 1 starting\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	std::cout << "task 1 finishing\n";
	});
	auto task2 = task1.ContinueWith<void>([](){
	std::cout << "task 2 starting\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	std::cout << "task 2 finishing\n";
	});

	auto task3 = Taskify::Task<void>([]() {
	std::cout << "task 3 starting\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	std::cout << "task 2 finishing\n";
	});
	task3.Start();
	std::cout << "task 3 started\n";
	task2.GetResult();
	task1.GetResult();
	task1.GetResult();
	task3.GetResult();

	// create vector for global functions
	/*std::vector<Taskify::Task<void>> tasks;
	tasks.push_back(Taskify::Task<void>([]() {
		std::cout << "slow task started\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		std::cout << "slow task finished\n";
	}));
	tasks.push_back(Taskify::Task<void>([]() {
		std::cout << "fast task started\n";
		std::cout << "fast task finished\n";
	}));
	tasks.push_back(Taskify::Task<void>([]() {
		std::cout << "medium task started\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		std::cout << "medium task finished\n";
	}));*/

	// TEST WaitAny
	/*std::cout << std::endl << "TESTING void WaitAny\n"<< std::endl;
	size_t i = Taskify::WaitAny(tasks);
	std::cout << "WaitAny returned index " << i << std::endl;*/

	// TEST WhenAny
	/*std::cout << std::endl << "TESTING void WhenAny\n" << std::endl;
	auto t = Taskify::WhenAny(tasks);
	std::cout << (t.HasStarted() ? "New running" : "New not running") << std::endl;
	for (size_t i = 0; i < tasks.size(); i++)
	{
	std::cout << i << (tasks[i].HasStarted() ? " running" : " not running") << std::endl;
	}
	t.GetResult();
	std::cout << "Got result\n";*/

	// TEST WaitAll
	/*std::cout << std::endl << "TESTING void WaitAll\n" << std::endl;
	Taskify::WaitAll(tasks);
	tasks[0].WaitTask();
	for (size_t i = 0; i < tasks.size(); i++)
	{
	tasks[i].GetResult();
	}
	std::cout << "Got all results\n" << std::endl;*/


	// TEST WhenAll
	/*std::cout << std::endl << "TESTING void WhenAll\n"<< std::endl;
	auto t = Taskify::WhenAll(tasks);
	std::cout << (t.HasStarted() ? "New running" : "New not running") << std::endl;
	t.GetResult();
	for (size_t i = 0; i < tasks.size(); i++)
	{
		std::cout << i << ":" << (!tasks[i].IsRunning() && tasks[i].HasStarted() ? "finished" : "not finished") << std::endl;
	}*/
}