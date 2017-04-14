
#ifndef VERIFIER_HPP
#define VERIFIER_HPP

#include <map>
#include <list>
#include <iostream>

class verifier
{
	using req = ACIO_client_request;
	std::map<uint64_t, uint64_t> model;
	std::map<req, std::list<float>> timing;
	float waiting_time;
	size_t reqs;
	float processing_time;
	size_t read_errors, timed_replies, replies_without_request;

public:
	verifier() {
		processing_time = waiting_time = 0;
		reqs = read_errors = timed_replies = 0;
		replies_without_request = 0;
	}

	void request (req*R, float dispatch, float ts) {
		req r = *R;
		if (r.type != req::write) r.value = 0;
		timing[r].push_back (ts);
		waiting_time += ts - dispatch;
		++reqs;
	}

	void reply (req*R, float ts) {
		req r = *R;

		if (r.type == req::read) {
			if (r.value != model[r.key]) {
				std::cerr << "read error: key " << r.key
				          << " should be " << model[r.key]
				          << " not " << r.value << std::endl;
				++read_errors;
			}
		} else {
			model[r.key] = r.value;
		}

		if (r.type != req::write) r.value = 0;
		if (timing[r].empty()) {
			++replies_without_request;
			return;
		}
		++timed_replies;
		processing_time += ts - timing[r].front();
		timing[r].pop_front();
	}

	void print_stats() {
		size_t dangling = 0;
		for (auto&r : timing) dangling += r.second.size();
		std::cout << "*** Verifier stats ***" << std::endl;
		std::cout << "Inconsistent values returned: " << read_errors << std::endl;
		std::cout << "Average request waiting time: "
		          << (waiting_time / reqs)
		          << 's' << std::endl;
		std::cout << "Average request processing time: "
		          << (processing_time / timed_replies)
		          << 's' << std::endl;
		std::cout << "Unreplied requests: " << dangling << std::endl;
		std::cout << "Unexpected replies: " << replies_without_request << std::endl;
	}
};

#endif
