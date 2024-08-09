#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <sys/syscall.h>
#include <mutex>
#include <unistd.h>
#include <thread>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

class ThreadPool{
private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> taskqueue_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic_bool stop_;
    const std::string threadtype_;                                 // 线程种类："IO"、"WORKS"

public:

	// 获取线程池的大小。
	size_t size();

    ThreadPool(size_t threadnum,const std::string& threadtype);
    void addtask(std::function<void()>task);
    ~ThreadPool();
    void stop();                // 停止服务。
};