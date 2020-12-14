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

#include <boost/test/unit_test.hpp>
#include "cli/simplescheduler.h"
#include "cli/pollingscheduler.h"
#include "cli/boostasioscheduler.h"

using namespace std;
using namespace cli;

BOOST_AUTO_TEST_SUITE(SchedulerSuite)

template <typename S>
void SchedulingTest()
{
    S scheduler;
    bool done = false;
    scheduler.Post( [&done](){ done = true; } );
    scheduler.ExecOne();
    BOOST_CHECK(done);
}

template <typename S>
void SameThreadTest()
{
    S scheduler;
    thread::id runThreadId;
    thread::id postThreadId;
    thread th( 
        [&]()
        {
            postThreadId = this_thread::get_id();
            scheduler.Post( 
                [&runThreadId]()
                { 
                    runThreadId = this_thread::get_id();
                }
            );
        }
    );
    th.join();
    scheduler.ExecOne();
    BOOST_CHECK_NE( runThreadId, postThreadId );
    BOOST_CHECK_EQUAL( runThreadId, this_thread::get_id() );
}

template <typename S>
void ExceptionTest()
{
    S scheduler;
    scheduler.Post( [](){ throw 42; } );
    BOOST_CHECK_THROW( scheduler.ExecOne(), int );
}

// start tests

BOOST_AUTO_TEST_CASE(Basics)
{
    SchedulingTest<SimpleScheduler>();
    SchedulingTest<PollingScheduler>();
    SchedulingTest<BoostAsioScheduler>();
}

BOOST_AUTO_TEST_CASE(SameThread)
{
    SameThreadTest<SimpleScheduler>();
    SameThreadTest<PollingScheduler>();
    SameThreadTest<BoostAsioScheduler>();
}

BOOST_AUTO_TEST_CASE(Exceptions)
{
    ExceptionTest<SimpleScheduler>();
    ExceptionTest<PollingScheduler>();
    ExceptionTest<BoostAsioScheduler>();
}

BOOST_AUTO_TEST_CASE(BoostAsioNonOwner)
{
    detail::BoostAsioLib::ContextType ioc;
    BoostAsioScheduler scheduler(ioc);
    bool done = false;
    scheduler.Post( [&done](){ done = true; } );
    ioc.run_one();
    BOOST_CHECK(done);
}


BOOST_AUTO_TEST_SUITE_END()