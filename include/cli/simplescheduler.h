/*******************************************************************************
 * CLI - A simple command line interface.
 * Copyright (C) 2016-2020 Daniele Pallastrelli
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************/

#ifndef CLI_LOCALSCHEDULER_H_
#define CLI_LOCALSCHEDULER_H_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "scheduler.h"

namespace cli
{

class SimpleScheduler : public Scheduler
{
public:
    SimpleScheduler() = default;
    ~SimpleScheduler()
    {
        Stop();
    }

    // non copyable
    SimpleScheduler(const SimpleScheduler&) = delete;
    SimpleScheduler& operator=(const SimpleScheduler&) = delete;

    void Stop()
    {
        std::lock_guard<std::mutex> lck (mtx);
        running = false;
        cv.notify_all();
    }
    void Run()
    {
        while( ExecOne() ) {};
    }
    void Post(const std::function<void()>& f) override
    {
        std::lock_guard<std::mutex> lck (mtx);
        tasks.push(f);
        cv.notify_all();
    }
    bool ExecOne()
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lck(mtx);
            cv.wait(lck, [this](){ return !running || !tasks.empty(); });
            if (!running)
                return false;
            task = tasks.front();
            tasks.pop();
        }

        if (task)
            task();

        return true;
    }
private:
    std::queue<std::function<void()>> tasks;
    bool running{ true };
    std::mutex mtx;
    std::condition_variable cv;
};

} // namespace cli

#endif // CLI_LOCALSCHEDULER_H_
