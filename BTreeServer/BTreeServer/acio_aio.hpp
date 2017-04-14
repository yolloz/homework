
#ifndef ACIO_AIO_HPP
#define ACIO_AIO_HPP

#include <aio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include <cmath>
#include <stdexcept>
#include <mutex>

#define LOCKING_NEEDED 1 //you may want to set this to 0 on systems with kernel AIO

template<class op, size_t block_size>
class aio
{
	bool clock_started;
	std::chrono::steady_clock::time_point clock_start;

	int fd;

#if LOCKING_NEEDED
	std::mutex sigops_mutex;
#endif
	std::set<op*> ops_signaled;

	std::vector<struct aiocb*> cb_vec;

	static void aio_signal_handler (int sig, siginfo_t*si, void*user_context) {
		op*o = (op*) si->si_value.sival_ptr;
#if LOCKING_NEEDED
		std::lock_guard<std::mutex> mtx (o->impl.parent->sigops_mutex);
#endif
		o->impl.parent->ops_signaled.insert (o);
	}
public:
	aio (const std::string&s) {
		clock_started = false;
		fd = open (s.c_str(), O_CREAT | O_RDWR /*| O_SYNC | O_DSYNC*/, 00600);
		if (fd < 0) {
			throw std::runtime_error ("can't open the file");
		}

		struct sigaction sa;
		sa.sa_sigaction = aio_signal_handler;
		sa.sa_flags = SA_RESTART | SA_SIGINFO;
		sigaction (SIGUSR1, &sa, NULL);
	}

	~aio() {
		close (fd);
	}

	bool resize (size_t n) {
		return !ftruncate (fd, block_size * n);
	}

	bool done() {
		return cb_vec.empty();
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
		friend class aio<op, block_size>;
		struct aiocb cb;
		size_t vec_pos;
		aio*parent;
	};

	bool read (op*o) {
		if (o->status != op::initial) return false;
		o->impl.parent = this;
		o->impl.vec_pos = cb_vec.size();
		cb_vec.push_back (& (o->impl.cb));

		memset (& (o->impl.cb), 0, sizeof (struct aiocb));
		o->impl.cb.aio_fildes = fd;
		o->impl.cb.aio_offset = block_size * o->block;
		o->impl.cb.aio_buf = o->data;
		o->impl.cb.aio_nbytes = block_size;
		o->impl.cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
		o->impl.cb.aio_sigevent.sigev_signo = SIGUSR1;
		o->impl.cb.aio_sigevent.sigev_value.sival_ptr = (void*) o;

		if (aio_read (&o->impl.cb)) return false;
		o->status = op::pending;
		return true;
	}

	bool write (op*o) {
		if (o->status != op::initial) return false;
		o->impl.parent = this;
		o->impl.vec_pos = cb_vec.size();
		cb_vec.push_back (& (o->impl.cb));

		memset (& (o->impl.cb), 0, sizeof (struct aiocb));
		o->impl.cb.aio_fildes = fd;
		o->impl.cb.aio_offset = block_size * o->block;
		o->impl.cb.aio_buf = o->data;
		o->impl.cb.aio_nbytes = block_size;
		o->impl.cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
		o->impl.cb.aio_sigevent.sigev_signo = SIGUSR1;
		o->impl.cb.aio_sigevent.sigev_value.sival_ptr = (void*) o;

		if (aio_write (&o->impl.cb)) return false;
		o->status = op::pending;
		return true;
	}

	bool finish (op*o) {
		if (o->status == op::error || o->status == op::done) return true;
		if (o->status != op::pending) return false;
		int ret = aio_error (&o->impl.cb);
		if (ret == EINPROGRESS) return false; //couldn't have been polled
		if (ret > 0) o->status = op::error;
		else o->status = op::done;
		//throw away the actual retval
		aio_return (&o->impl.cb);

		size_t vec_pos = o->impl.vec_pos;
		cb_vec[vec_pos] = cb_vec.back();
		cb_vec.pop_back();
#if LOCKING_NEEDED
		std::lock_guard<std::mutex> mtx (sigops_mutex);
#endif
		ops_signaled.erase (o);
		return true;
	}

	void poll (std::list<op*>&ops, float time) {
		struct timespec ts;
		if (!ops_signaled.empty()) goto output;
		ts.tv_sec = (time_t) floorf (time);
		ts.tv_nsec = (unsigned long) floorf ( (time - ts.tv_sec) * 1000000000);
		if (cb_vec.empty()) //suspend doesn't work with nitems==0 !
			std::this_thread::sleep_for
			(std::chrono::duration<float> (time));
		else
			aio_suspend (cb_vec.data(), cb_vec.size(), &ts);
	output:
		ops.clear();
#if LOCKING_NEEDED
		std::lock_guard<std::mutex> mtx (sigops_mutex);
#endif
		ops.insert (ops.begin(), ops_signaled.begin(), ops_signaled.end());
	}
};
#endif
