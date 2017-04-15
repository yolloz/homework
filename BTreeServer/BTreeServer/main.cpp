
#include "acio.hpp"
#include "acio_mock.hpp"
#include "verifier.hpp"
#include "seq_client.hpp"
#include "burst_client.hpp"
#include "btserver.hpp"

#include <iostream>

/*
 * all test params are filled in here
 */
//default test -- uses a mocked in-memory AIO class that simulates the delays
using IO = ACIO<mock, seq_client<100, 10, 30>, verifier>;

//expected test params
//using IO = ACIO<mock, burst_client<10000, 10000, 3, 10000000> /*,verifier*/>;

//external storage support (only on systems with aio, may fail in very interesting ways)
//#include "acio_aio.hpp"
//using IO = ACIO<aio, burst_client<100000, 10000, 1, 1000000000> /*,verifier*/>;


/*
 * server implementation
 */
void run_dummy_demo (IO&);

void run_btree_server (IO&io)
{
	//TODO put your server implementation here, instead of the demo
	//run_dummy_demo (io);
	auto tree = BTree<IO::block_size>(io);

	size_t max = 1000;
	
	for (size_t i = 0; i < max; i++)
	{
		tree.Write(i, i+10000);
	}
	size_t i = 0;
	while (i < max) {
		if (tree.Read(i) != i + 10000) {
			std::cout << "ERR!  Key: " << i << ", got: " << tree.Read(i) << std::endl;
		}
		if (i % 50 == 0) std::cout << i << std::endl;
		i++;
	}

	std::cout << "Finished Loading" << std::endl;

	for (i = 0; i < max; i+=50)
	{
		tree.Erase(i);
		if (tree.Read(504) != 10504) {
			i = i;
		}
	}

	std::cout << "Finished erasing" << std::endl;

	i = 0;
	bool correct = true;
	/*while (i < max && correct) {		
		if (i % 50 == 0) {
			correct = tree.Read(i) == 0;
			std::cout << i << std::endl;
		}
		else {
			correct = tree.Read(i) == i+10000;
		}
		i++;
	}*/
	uint64_t rtc;
	while (i < max) {
		rtc = tree.Read(i);
		if(rtc != i + 10000){
			if (i % 50 == 0) {
				if (rtc == 0) {
					i++;
					continue;
				}
			}
			std::cout << "ERR!  Key: " << i << ", got: " << rtc << std::endl;
		}
		i++;
	}
	std::cout << "Last correct: " << --i << std::endl;
}

/*
 * entry point
 */
int main (int argc, char**argv)
{
	std::string fn = "dummy.database";
	if (argc > 1) {
		fn = argv[1];
	}
	IO io (fn);

	run_btree_server (io);
	io.print_stats();
	return 0;
}


/*
 * The dummy demo implementation can only handle 1 block worth of entries (ie.
 * around 512 stored pairs), works in a completely synchronous way and ignores
 * most errors.
 */

#include <array>

const size_t block_entries = IO::block_size / (2 * sizeof (uint64_t));
using block_array = std::array<uint64_t, 2 * block_entries>;

bool sync_op (IO&io, IO::op*o)
{
	std::list<IO::op*> ops;
	//loop until the operation is done, ignore pending requests
	while (io.poll (ops, 1.0, false))
		for (auto&i : ops) if (i == o) {
				io.finish (o);
				return true;
			}
	return false;
}

bool sync_write (IO&io, size_t block, block_array&b)
{
	//call write and wait for it to finish
	IO::op o (block, b.data());
	io.write (&o);
	return sync_op (io, &o);
}

bool sync_read (IO&io, size_t block, block_array&b)
{
	IO::op o (block, b.data());
	io.read (&o);
	return sync_op (io, &o);
}

void run_dummy_demo (IO&io)
{
	size_t i;
	//allocate one block
	if (!io.resize (1)) {
		std::cerr << "resize failed" << std::endl;
		return;
	}

	{
		//write "empty" data in that block
		block_array b;
		for (i = 0; i < block_entries; ++i) {
			b[2 * i] = -1; //"empty"
			b[2 * i + 1] = 0;
		}
		sync_write (io, 0, b);
	}

	std::list<IO::op*> ops;
	//wait for any request or finished operation, max 0.1s
	while (io.poll (ops, 0.1, true)) {
		std::cerr << "current timestamp: " << io.timestamp() << std::endl;
		IO::req r;
		//if there's no request available, retry
		if (!io.recv_request (&r)) continue;
		std::cerr << "got a request!" << std::endl;
		switch (r.type) {
		case IO::req::write: {
			//handle client writing into the database
			block_array b;
			//read the block from disk
			sync_read (io, 0, b);
			//find a place where the key's value is saved
			for (i = 0; i < block_entries; ++i)
				if (b[2 * i] == r.key) {
					b[2 * i] = r.key;
					b[2 * i + 1] = r.value;
					break;
				}
			if (i == block_entries)
				//if it was not found, find an empty spot
				for (i = 0; i < block_entries; ++i)
					if (b[2 * i] == (uint64_t) -1) {
						b[2 * i] = r.key;
						b[2 * i + 1] = r.value;
						break;
					}
			std::cerr << "write " << r.key << " = " << r.value << std::endl;
			//write the resulting block back
			sync_write (io, 0, b);
			//send the confirmation
			io.send_reply (&r);
			break;
		}
		case IO::req::read: {
			block_array b;
			//handling read -- get the block from disk first
			sync_read (io, 0, b);
			r.value = 0;
			//find the key
			for (i = 0; i < block_entries; ++i)
				if (b[2 * i] == r.key) {
					//return its value
					r.value = b[2 * i + 1];
					break;
				}
			std::cerr << "read " << r.key << " --> " << r.value << std::endl;
			//send the reply with the value
			io.send_reply (&r);
			break;
		}
		case IO::req::erase:
			block_array b;
			sync_read (io, 0, b);
			//in erasing case, find the key
			for (i = 0; i < block_entries; ++i)
				if (b[2 * i] == r.key) {
					//erase it
					b[2 * i] = -1;
					b[2 * i + 1] = 0;
					break;
				}
			//write it back
			sync_write (io, 0, b);
			r.value = 0;
			std::cerr << "erase " << r.key << std::endl;
			//and send the confirmation.
			io.send_reply (&r);
			break;
		default:
			break;
		}
	}

	//if poll() returned false, it means that there are no more pending
	//requests and all I/O operations have finished.
	std::cerr << "polling ended, terminating" << std::endl;
}
