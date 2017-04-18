
#ifndef ACIO_HPP
#define ACIO_HPP

#include <cstdint>
#include <string>
#include <list>
#include <iostream>

struct ACIO_client_request {
	enum req_type {
		read,
		write,
		erase
	};
	req_type type;
	uint64_t key, value;

	ACIO_client_request() = default;

	ACIO_client_request (uint64_t key, uint64_t value)
		: key (key),
		  value (value) {}

	/* helper for indexing by request */
	bool operator< (const ACIO_client_request&a) const {
		if (type < a.type) return true;
		if (type > a.type) return false;
		if (key < a.key) return true;
		if (key > a.key) return false;
		return value < a.value;
	}
};

class dummy_verifier
{
public:
	void request (ACIO_client_request*r, float ts) {}
	void reply (ACIO_client_request*r, float ts) {}
	void print_stats() {}
};

template<template<class, size_t> class ImplT, class Client, class Verifier = dummy_verifier>
class ACIO
{
public:
	static const size_t block_size = 1024;
	class op;
	using Impl = ImplT<op, block_size>;
	using req = ACIO_client_request;

	class op
	{
	public:
		typename Impl::op_impl impl;

		size_t block;
		void* data;

		enum op_status {
			initial,
			done,
			pending,
			error
		};

		op_status status;

		op (size_t block, void* data)
			: block (block),
			  data (data),
			  status (initial) {}
	};

private:
	Impl impl;
	Client client;
	Verifier verifier;
	float last_timestamp;

public:

	ACIO() = delete;
	ACIO (const std::string&fn) : impl (fn) {}
	ACIO (const ACIO&) = delete;
	ACIO (ACIO&&) = delete;
	ACIO& operator= (const ACIO&) = delete;
	ACIO& operator= (ACIO&&) = delete;

	bool resize (size_t n) {
		return impl.resize (n);
	}

	float timestamp() {
		return impl.timestamp();
	}

	bool read (op*o) {
		return impl.read (o);
	}
	bool write (op*o) {
		return impl.write (o);
	}
	bool finish (op*o) {
		return impl.finish (o);
	}

	bool poll (std::list<op*>& ops, float time, bool requests = true) {
		if (client.done() && impl.done()) {
			last_timestamp = timestamp();
			return false;
		}
		impl.poll (ops,
		           requests ? std::min
		           (client.wait_time (timestamp()),
		            time) : time);
		return true;
	}

	bool recv_request (req*r) {
		auto ts = timestamp();
		float dispatch;
		if (client.recv_request (r, dispatch, ts)) {
			verifier.request (r, dispatch, ts);
			return true;
		}
		return false;
	}

	void send_reply (req*r) {
		auto ts = timestamp();
		verifier.reply (r, ts);
		client.send_reply (r, ts);
	}

	void print_stats () {
		verifier.print_stats();
		std::cout << "Total run time: " << last_timestamp << 's' << std::endl;
	}
};

#endif
