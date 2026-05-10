/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Sleep.h"
#include "Awl/CppStd/Thread.h"

#include <chrono>
#include <functional>
namespace awl
{
    //Stops a test when a timeout has elapsed.
    class watch_dog
    {
    public:

        watch_dog(std::stop_token token, std::chrono::nanoseconds timeout, std::function<void()> log) :
            _token(token),
            _timeout(timeout),
            _thread(std::bind(&watch_dog::thread_proc, this, std::placeholders::_1)),
            _callback(_token, std::bind(&watch_dog::callback_proc, this)),
            _log(log)
        {}

        std::stop_token get_token() const
        {
            return _source.get_token();
        }

    private:

        //Stop the test when the timout has elapsed.
        void thread_proc(std::stop_token token)
        {
            awl::sleep_for(_timeout, token);

            if (!token.stop_requested())
            {
                //The timeout has elapsed.
                _log();
            }

            _source.request_stop();
        }

        //Stop the test when the user pressed "Cancel" button.
        void callback_proc()
        {
            _source.request_stop();
        }

        std::stop_source _source;

        std::stop_token _token;

        std::chrono::nanoseconds _timeout;

        std::jthread _thread;

        std::stop_callback<std::function<void()>> _callback;

        std::function<void()> _log;
    };
}
