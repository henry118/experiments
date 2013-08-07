#ifndef __IOSERVICE_H__
#define __IOSERVICE_H__

#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <stdio.h>
#include <stdexcept>

namespace shfe {

class IoService : private boost::noncopyable {
    typedef boost::unique_lock<boost::mutex> LOCK;
public:
    IoService() : alive(false), starting(false) {
    }
    ~IoService() {
        stop();
    }
    template<typename FUNC>
    void async_run(FUNC func) {
        io.post(func);
    }
    void start() {
        LOCK lock(mut);
        if (!alive) {
            if (!starting) {
                work.reset(new boost::asio::io_service::work(io));
                thr = boost::thread(boost::bind(&IoService::io_service_thread, this));
                starting = true;
            }
            while(!alive) {
                cond.wait(lock);
            }
        }
    }
    void stop() {
        LOCK lock(mut);
        if (alive) {
            alive = false;
            starting = false;
            work.reset();
            thr.join();
            io.reset();
            fprintf(stderr, "[IO] IO Service thread terminated\n");
        }
    }
private:
    void io_service_thread() {
        fprintf(stderr, "[IO] IO Service thread started\n");
        {
            LOCK lock(mut);
            starting = false;
            alive = true;
            cond.notify_all();
        }
        while (alive) {
            try {
                io.run();
            } catch (std::exception& e) {
                fprintf(stderr, "[IO] ERROR: IO Service absorbed exception: %s\n", e.what());
            } catch (...) {
                fprintf(stderr, "[IO] ERROR: IO Service absorbed unknown exception\n");
            }
        }
    }
private:
    volatile bool alive;
    bool starting;
    boost::thread thr;
    boost::asio::io_service io;
    boost::scoped_ptr<boost::asio::io_service::work> work;
    boost::mutex mut;
    boost::condition_variable cond;
};

} // namespace shfe {

#endif // __IOSERVICE_H__
