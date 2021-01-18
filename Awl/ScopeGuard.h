#pragma once

#include <utility>

namespace awl
{
    template<typename Lambda>
    class scope_guard
    {
    public:

        scope_guard(Lambda f, bool e = true) : free(std::move(f)), engaged(e)
        {
        }

        scope_guard(const scope_guard &) = delete;

        scope_guard & operator = (const scope_guard &) = delete;

        scope_guard(scope_guard && other) : free(std::move(other.free))
        {
            other.engaged = false;
        }

        scope_guard & operator = (scope_guard && other)
        {
            free = std::move(other.free);
            other.engaged = false;
            return *this;
        }

        ~scope_guard()
        {
            if (engaged)
            {
                free();
            }
        }

        void release()
        {
            engaged = false;
        }

    private:

        std::decay_t<Lambda> free;

        bool engaged = true;
    };

    template <class Lambda>
    inline scope_guard<Lambda> make_scope_guard(Lambda free, bool engaged = true)
    {
        return scope_guard<Lambda>(std::move(free), engaged);
    }

    template <class Init, class Free>
    inline scope_guard<Free> make_scope_guard(Init init, Free free, bool engaged = true)
    {
        if (engaged)
        {
            init();
        }

        return make_scope_guard(std::move(free), engaged);
    }
}
