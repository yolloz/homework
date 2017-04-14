

#ifndef BURST_CLIENT_HPP
#define BURST_CLIENT_HPP

/* basically the same as seq_client,
 * only sends the requests in bursts of given size */

#include <cmath>
#include <random>

template<size_t n_bursts, size_t burst_size, size_t freq, size_t max_index>
class burst_client
{
	static const size_t n_requests = n_bursts * burst_size;

	size_t reqs;
	std::default_random_engine generator;
	std::uniform_int_distribution<size_t> dist_type, dist_idx, dist_value;
public:
	burst_client()
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
			return burst_size * (1 + floorf (ts * freq)) - reqs;
		else return 0;
	}

	float wait_time (float ts) {
		if (requests_available (ts)) return 0; //available immediately
		else return (reqs / burst_size) / (float) freq - ts;
	}

	using req = ACIO_client_request;
	bool recv_request (req*r, float&dispatched, float ts) {
		if (requests_available (ts)) {
			size_t sample = dist_type (generator);
			if (sample > reqs) r->type = req::write;
			else {
				sample = dist_type (generator);
				if (sample / 2 > n_requests - reqs) r->type = req::erase;
				else r->type = req::read;
			}
			r->key = dist_idx (generator);
			r->value = r->type == req::write ? dist_value (generator) : 0;
			dispatched = (reqs / burst_size) / (float) freq;
			++reqs;
			return true;
		}
		return false;
	}

	void send_reply (req*r, float ts) {
		//nothing.
	}
};

#endif
