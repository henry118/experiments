#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <iostream>

template<size_t N>
class ThreadPool {
    boost::asio::io_service ios;
    boost::asio::io_service::work work;
    boost::thread pool[N];
private:
    static void threadproc(boost::asio::io_service & ios) {
        for (;;) {
            try {
                ios.run();
                break;
            } catch (std::exception & e) {
                std::cout << e.what() << std::endl;
            }
        }
    }
public:
    ThreadPool() : work(ios) {
        for(int i = 0; i < N; i++) {
            pool[i] = boost::thread(boost::bind(&ThreadPool::threadproc, boost::ref(ios)));
        }
    }
    ~ThreadPool() {
        ios.stop();
        for(int i = 0; i < N; i++) {
            pool[i].join();
        }
    }
    template<class F>
    void queue_work(F functor) {
        ios.post(functor);
    }
};

#endif //__THREAD_POOL_H__
