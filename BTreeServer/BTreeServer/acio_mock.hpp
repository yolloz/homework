
#ifndef ACIO_MOCK_HPP
#define ACIO_MOCK_HPP

#include "acio.hpp"
#include <vector>
#include <array>
#include <set>
#include <cstring>
#include <chrono>
#include <thread>
#include <random>
#include <map>
#include <cmath>

template<class op, size_t block_size>
class mock
{
	//tunable
	const float average_frequency = 100;

	using block = std::array<uint8_t, block_size>;
	std::vector<block> data;

	bool clock_started;
	std::chrono::steady_clock::time_point clock_start;

	std::default_random_engine generator;
	std::exponential_distribution<> distribution;

	std::map<float, op*> pending_ops;

	void run_op (op*o) {
		if (o->impl.sched > timestamp()) return;
		if (o->status != op::pending) return;

		if (o->block >= data.size()) {
			o->status = op::error;
			return;
		}

		if (o->impl.read)
			std::memcpy (o->data, data[o->block].data(), block_size);
		else
			std::memcpy (data[o->block].data(), o->data, block_size);
		o->status = op::done;
	}

	float get_sched() {
		float sched;
		do {
			sched = timestamp()
			        + distribution (generator)
			        * sqrtf(1 + pending_ops.size());
		} while (pending_ops.count (sched));
		return sched;
	}
public:
	mock (const std::string&s)
		: distribution (average_frequency) {
		clock_started = false;
	}

	bool resize (size_t n) {
		data.resize (n);
		return true;
	}

	bool done() {
		return pending_ops.empty();
	}

	float timestamp() {
		using namespace std::chrono;
		if (!clock_started) {
			clock_start = steady_clock::now();
			clock_started = true;
			return 0;
		} else {
			steady_clock::time_point t = steady_clock::now();
			return duration_cast<duration<double>>
			       (t - clock_start).count();
		}
	}

	class op_impl
	{
		friend class mock<op, block_size>;
		bool read; //otherwise it's write
		float sched;
	};

	bool read (op*o) {
		float sched = get_sched();

		o->impl.read = true;
		o->impl.sched = sched;
		o->status = op::pending;
		pending_ops[sched] = o;
		return true;
	}

	bool write (op*o) {
		float sched = get_sched();
		o->impl.read = false;
		o->impl.sched = sched;
		o->status = op::pending;
		pending_ops[sched] = o;
		return true;
	}

	bool finish (op*o) {
		if (o->status != op::error && o->status != op::done) return false;
		pending_ops.erase (o->impl.sched);
		return true;
	}

	void poll (std::list<op*>&ops, float time) {
		ops.clear();
		float now = timestamp();
		for (auto&i : pending_ops) {
			if (i.first > now) break;
			run_op (i.second);
			ops.push_back (i.second);
		}

		if (!pending_ops.empty())
			time = std::min (time, std::max (0.0f, pending_ops.begin()->first - now));

		std::this_thread::sleep_for (std::chrono::duration<float> (time));
	}
};

#endif
