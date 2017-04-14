
#ifndef SEQ_CLIENT_HPP
#define SEQ_CLIENT_HPP

#include <cmath>
#include <random>

template<size_t n_requests, size_t freq, size_t max_index>
class seq_client
{
	size_t reqs;
	std::default_random_engine generator;
	std::uniform_int_distribution<size_t> dist_type, dist_idx, dist_value;
public:
	seq_client()
		: dist_type (0, n_requests),
		  dist_idx (0, max_index),
		  dist_value (0, (uint64_t) - 1) {
		reqs = 0;
	}

	bool done() {
		return reqs >= n_requests;
	}

	size_t requests_available (float ts) {
		if (reqs < n_requests)
			return 1 + floorf (ts * freq) - reqs;
		else return 0;
	}

	float wait_time (float ts) {
		if (requests_available (ts)) return 0; //available immediately
		else return reqs / (float) freq - ts;
	}


	using req = ACIO_client_request;
	bool recv_request (req*r, float&dispatched, float ts) {
		if (requests_available (ts)) {
			/* produce requests in 3 waves, start with lot of
			 * writes, mostly reads in the middle, and erases on
			 * the end */
			size_t sample = dist_type (generator);
			if (sample > reqs) r->type = req::write;
			else {
				sample = dist_type (generator);
				if (sample / 2 > n_requests - reqs) r->type = req::erase;
				else r->type = req::read;
			}
			r->key = dist_idx (generator);
			r->value = r->type == req::write ? dist_value (generator) : 0;
			dispatched = reqs / (float) freq;
			++reqs;
			return true;
		}
		return false;
	}

	void send_reply (req*r, float ts) {
		//not much, verification is elsewhere
	}
};

#endif
