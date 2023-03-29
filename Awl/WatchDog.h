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

        watch_dog(std::stop_token token, std::chrono::nanoseconds timeout) :
            m_token(token),
            m_timeout(timeout),
            m_thread(std::bind(&watch_dog::thread_proc, this, std::placeholders::_1)),
            m_callback(m_token, std::bind(&watch_dog::callback_proc, this))
        {
        }

        std::stop_token get_token() const
        {
            return m_source.get_token();
        }

    private:

        //Stop the test when the timout has elapsed.
        void thread_proc(std::stop_token token)
        {
            awl::sleep_for(m_timeout, token);

            m_source.request_stop();
        }

        //Stop the test when the user pressed "Cancel" button.
        void callback_proc()
        {
            m_source.request_stop();
        }

        std::stop_source m_source;

        std::stop_token m_token;

        std::chrono::nanoseconds m_timeout;

        std::jthread m_thread;

        std::stop_callback<std::function<void()>> m_callback;
    };
}
