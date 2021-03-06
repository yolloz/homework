// MICHAL JURCO

#ifndef TASK_CLASS_MJ_
#define TASK_CLASS_MJ_

#include <future>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <vector>

namespace Taskify {
	template<class Ret>
	// Represents a task which should be executed in a different thread with delayed start
	class Task
	{
	public:
		Task(std::function<Ret(void)> f) {
			_wrapper = [=](std::promise<Ret>&& promise, std::atomic<bool> * running) {
				running->store(true);
				Ret result = f();
				promise.set_value(result);
				running->store(false);
			};
			_hasStarted = std::make_shared<std::atomic<bool>>();
			_hasStarted->store(false);
			_isRunning = std::make_shared<std::atomic<bool>>();
			_isRunning->store(false);
			_future = std::make_shared<std::shared_future<Ret>>();
		}		

		Task(const Task<Ret> & other) {
			_wrapper = other._wrapper;
			_future = other._future;
			_isRunning = other._isRunning;
			_hasStarted = other._hasStarted;
		}

		template<class PRet>
		Task(std::function < Ret(PRet) > f, Task<PRet> previousTask) {
			_wrapper = [=](std::promise<Ret>&& promise, std::atomic<bool> * running) mutable {
				PRet previousResult = previousTask.GetResult();
				Ret result = f(previousResult);
				promise.set_value(result);
			};
			_hasStarted = std::make_shared<std::atomic<bool>>();
			_hasStarted->store(false);
			_isRunning = std::make_shared<std::atomic<bool>>();
			_isRunning->store(false);
			_future = std::make_shared<std::shared_future<Ret>>();
		}

		// Starts the execution of task, if task is already running, nothing happens
		void Start() {
			if (_hasStarted->load() == false) {
				bool exp = false;
				if (_hasStarted->compare_exchange_weak(exp, true)) {
					std::promise<Ret> promise;
					auto tmp = promise.get_future();
					*_future = tmp.share();
					_thread = std::thread(_wrapper, std::move(promise), _isRunning.get());
				}
			}
		}

		// Waits for the task to finish and returns result
		Ret GetResult() {
			if (_hasStarted->load() == false) {
				Start();
			}
			return _future->get();
		}
		// Waits for the task to finish
		void WaitTask() {
			if (_hasStarted->load() == false) {
				Start();
			}
			_future->wait();
		}

		// Chains new task after current task, which will be executed right after current task ends
		template<class CRet>
		Task<CRet> ContinueWith(std::function<CRet(Ret)> f) {
			auto task = Task<CRet>(f, *this);
			return task;
		}

		~Task() {
			if (_thread.joinable()) { _thread.join(); }
		}

		// Returns value indicating whether the task is running or not
		bool IsRunning() {
			return _isRunning->load();
		}

		// Returns value indicating whether the task has been started or not
		bool HasStarted() {
			return _hasStarted->load();
		}
	private:
		std::function<void(std::promise<Ret>&&, std::atomic<bool>*)> _wrapper;	// wraps tasks' function so it can be run later
		std::shared_ptr<std::shared_future<Ret>> _future;	// future for storing result
		std::thread _thread;	// tasks' thread
		std::shared_ptr<std::atomic<bool>> _hasStarted;
		std::shared_ptr<std::atomic<bool>> _isRunning;		
	};

	template<class Ret>
	// Blocks the thread until all given tasks are finished
	void WaitAll(std::vector<Task<Ret>>& tasks) {
		for (size_t i = 0; i < tasks.size(); i++)
		{
			tasks[i].Start();
		}
		for (size_t i = 0; i < tasks.size(); i++)
		{
			tasks[i].WaitTask();
		}
	}

	template<class Ret>
	// Blocks the thread until any of the given tasks finishes, returns index of finished task
	size_t WaitAny(std::vector<Task<Ret>>& tasks) {
		for (size_t i = 0; i < tasks.size(); i++)
		{
			tasks[i].Start();
		}
		while (true) {
			for (size_t i = 0; i < tasks.size(); i++)
			{
				if (tasks[i].HasStarted() && !tasks[i].IsRunning()) {
					return i;
				}
			}
			std::this_thread::yield();
		}
	}

	template<class Ret>
	// Creates task that waits for all given tasks to finish and returns all their results
	Task<std::vector<Ret>> WhenAll(std::vector<Task<Ret>>& tasks) {
		return Task<std::vector<Ret>>([&]() {
			for (size_t i = 0; i < tasks.size(); i++)
			{
				tasks[i].Start();
			}
			std::vector<Ret> result;
			for (size_t i = 0; i < tasks.size(); i++)
			{
				result.push_back(tasks[i].GetResult());
			}
			return result;
		});
	}

	template<class Ret>
	// Returns task that waits for any of the given tasks to finish and returns it's result
	Task<Ret> WhenAny(std::vector<Task<Ret>>& tasks) {
		return Task<Ret>([&]() {
			for (size_t i = 0; i < tasks.size(); i++)
			{
				tasks[i].Start();
			}
			while (true) {
				for (size_t i = 0; i < tasks.size(); i++)
				{
					if (tasks[i].HasStarted() && !tasks[i].IsRunning()) {
						return tasks[i].GetResult();
					}
				}
				std::this_thread::yield();
			}
		});
	}

	template <class Ret>
	// Creates and starts new task immediately
	Task<Ret> StartNew(std::function<Ret(void)> f) {
		auto task = Task<Ret>(f);
		task.Start();
		return task;
	}

	/* VOID SPECIALIZATION */


	template<>
	class Task<void>
	{
	public:
		Task(std::function<void(void)> f) {
			_wrapper = [=](std::promise<void>&& promise, std::atomic<bool> * running) {
				running->store(true);
				f();
				promise.set_value();
				running->store(false);
			};
			_hasStarted = std::make_shared<std::atomic<bool>>();
			_hasStarted->store(false);
			_isRunning = std::make_shared<std::atomic<bool>>();
			_isRunning->store(false);
			_future = std::make_shared<std::shared_future<void>>();
		}

		Task(const Task<void> & other) {
			_wrapper = other._wrapper;
			_future = other._future;
			_isRunning = other._isRunning;
			_hasStarted = other._hasStarted;
		}

		template<class PRet>
		Task(std::function < void(PRet) > f, Task<PRet> previousTask) {
			_wrapper = [=](std::promise<void>&& promise, std::atomic<bool> * running) mutable {
				PRet previousResult = previousTask.GetResult();
				f(previousResult);
				promise.set_value();
			};
			_hasStarted = std::make_shared<std::atomic<bool>>();
			_hasStarted->store(false);
			_isRunning = std::make_shared<std::atomic<bool>>();
			_isRunning->store(false);
			_future = std::make_shared<std::shared_future<void>>();
		}

		Task(std::function < void(void) > f, Task<void> previousTask) {
			_wrapper = [=](std::promise<void>&& promise, std::atomic<bool> * running) mutable {
				previousTask.GetResult();
				f();
				promise.set_value();
			};
			_hasStarted = std::make_shared<std::atomic<bool>>();
			_hasStarted->store(false);
			_isRunning = std::make_shared<std::atomic<bool>>();
			_isRunning->store(false);
			_future = std::make_shared<std::shared_future<void>>();
		}

		void Start() {
			if (_hasStarted->load() == false) {
				bool exp = false;
				if (_hasStarted->compare_exchange_weak(exp, true)) {
					std::promise<void> promise;
					auto tmp = promise.get_future();
					*_future = tmp.share();
					_thread = std::thread(_wrapper, std::move(promise), _isRunning.get());
				}
			}
		}

		void GetResult() {
			if (_hasStarted->load() == false) {
				Start();
			}
			_future->get();
		}

		void WaitTask() {
			if (_hasStarted->load() == false) {
				Start();
			}
			_future->wait();
		}

		template<class CRet>
		Task<CRet> ContinueWith(std::function<CRet(void)> f) {
			Task<CRet> task = Task<CRet>(f, *this);
			return task;
		}

		~Task() {
			if (_thread.joinable()) { _thread.join(); }
		}

		bool IsRunning() {
			return _isRunning->load();
		}

		bool HasStarted() {
			return _hasStarted->load();
		}
	private:
		std::function<void(std::promise<void>&&, std::atomic<bool>*)> _wrapper;
		std::shared_ptr<std::shared_future<void>> _future;
		std::thread _thread;
		std::shared_ptr<std::atomic<bool>> _hasStarted;
		std::shared_ptr<std::atomic<bool>> _isRunning;
	};

	Task<void> WhenAll(std::vector<Task<void>>& tasks) {
		return Task<void>([&]() {
			for (size_t i = 0; i < tasks.size(); i++)
			{
				tasks[i].Start();
			}
			for (size_t i = 0; i < tasks.size(); i++)
			{
				tasks[i].GetResult();
			}
			return;
		});
	}

	template<>
	Task<void> WhenAny(std::vector<Task<void>>& tasks) {
		return Task<void>([&]() {
			for (size_t i = 0; i < tasks.size(); i++)
			{
				tasks[i].Start();
			}
			while (true) {
				for (size_t i = 0; i < tasks.size(); i++)
				{
					if (tasks[i].HasStarted() && !tasks[i].IsRunning()) {
						return;
					}
				}
				std::this_thread::yield();
			}
		});
	}
}
#endif
