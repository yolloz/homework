// MICHAL JURCO

#ifndef TASK_CLASS_MJ_
#define TASK_CLASS_MJ_

#include <future>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>

namespace Taskify {
	template<class Ret>
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
		}

		Task(const Task<Ret> & other) {
			_wrapper = other._wrapper;
			_future = other._future;
			_isRunning = other._isRunning;
			_hasStarted = other._hasStarted;
		}

		void Start() {
			if (_hasStarted->load() == false) {
				bool exp = false;
				if (_hasStarted->compare_exchange_weak(exp, true)) {
					std::promise<Ret> promise;
					auto tmp = promise.get_future();
					_future = tmp.share();
					_thread = std::thread(_wrapper, std::move(promise), _isRunning.get());
				}
			}
		}
		Ret GetResult() {
			if (_hasStarted->load() == false) {
				Start();
			}
			_future.wait();
			if (_future.valid()) {
				return _future.get();
			}
		}

		template<class CRet>
		Task<CRet> ContinueWith(std::function<CRet(Ret)> f) {
			auto task = Task<CRet>(f, *this);
			return task;
		}
		~Task() {
			if (_thread.joinable()) { _thread.join(); }
		}
	private:
		std::function<void(std::promise<Ret>&&, std::atomic<bool>*)> _wrapper;
		std::shared_future<Ret> _future;
		std::thread _thread;
		std::shared_ptr<std::atomic<bool>> _hasStarted;
		std::shared_ptr<std::atomic<bool>> _isRunning;
	};

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
		}

		template<class PRet>
		Task(std::function < void(PRet) > f, Task<PRet> previousTask) {
			_wrapper = [=](std::promise<Ret>&& promise, std::atomic<bool> * running) mutable {
				PRet previousResult = previousTask.GetResult();
				f(previousResult);
				promise.set_value();
			};
			_hasStarted = std::make_shared<std::atomic<bool>>();
			_hasStarted->store(false);
			_isRunning = std::make_shared<std::atomic<bool>>();
			_isRunning->store(false);
		}

		Task(const Task<void> & other) {
			_wrapper = other._wrapper;
			_isRunning = other._isRunning;
			_hasStarted = other._hasStarted;
		}

		void Start() {
			if (_hasStarted->load() == false) {
				bool exp = false;
				if (_hasStarted->compare_exchange_weak(exp, true)) {
					std::promise<void> promise;
					auto tmp = promise.get_future();
					_future = tmp.share();
					_thread = std::thread(_wrapper, std::move(promise), _isRunning.get());
				}
			}
		}
		void GetResult() {
			if (_hasStarted->load() == false) {
				Start();
			}
			_future.wait();
			_future.get();
		}

		template<class CRet>
		Task<CRet> ContinueWith(std::function<CRet(void)> f) {
			auto task = Task<CRet>(f, *this);
			return task;
		}
		~Task() {
			if (_thread.joinable()) { _thread.join(); }
		}
	private:
		std::function<void(std::promise<void>&&, std::atomic<bool>*)> _wrapper;
		std::shared_future<void> _future;
		std::thread _thread;
		std::shared_ptr<std::atomic<bool>> _hasStarted;
		std::shared_ptr<std::atomic<bool>> _isRunning;
	};
}
#endif
