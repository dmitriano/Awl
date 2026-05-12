/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/ISignal.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <concepts>
#include <utility>
#include <vector>

namespace awl
{
    template <class... Args>
    class Source :
        public ISignal<Args...>
    {
    private:

        using Slot = typename ISignal<Args...>::Slot;

        struct SlotRecord
        {
            Slot slot;
            bool removed = false;
        };

    public:

        using container_type = std::vector<SlotRecord>;
        using ISignal<Args...>::subscribe;
        using ISignal<Args...>::unsubscribe;

        void subscribe(Slot slot) override
        {
            const auto it = std::find_if(_slots.begin(), _slots.end(),
                [&slot](const SlotRecord& record)
                {
                    return !record.removed && record.slot == slot;
                });

            if (it == _slots.end())
            {
                _slots.push_back(SlotRecord{ std::move(slot) });
            }
        }

        bool unsubscribe(const Slot& slot) override
        {
            const auto it = std::find_if(_slots.begin(), _slots.end(),
                [&slot](const SlotRecord& record)
                {
                    return !record.removed && record.slot == slot;
                });

            if (it == _slots.end())
            {
                return false;
            }

            if (_emitting)
            {
                it->removed = true;
            }
            else
            {
                eraseRecord(it);
            }

            return true;
        }

        template<typename ...Params>
        void emit(const Params&... args) const
            requires (std::invocable<Slot&, const Params&...>)
        {
            assert(!_emitting);

            EmitGuard guard(*this);

            for (SlotRecord& record : _slots)
            {
                if (record.removed)
                {
                    continue;
                }

                auto slot_guard = record.slot.lock();

                if (slot_guard)
                {
                    slot_guard(args...);
                }
                else
                {
                    record.removed = true;
                }
            }
        }

        void clear() noexcept
        {
            container_type().swap(_slots);
        }

        bool empty() const noexcept
        {
            return size() == 0;
        }

        std::size_t size() const noexcept
        {
            return static_cast<std::size_t>(std::count_if(_slots.begin(), _slots.end(),
                [](const SlotRecord& record)
                {
                    return !record.removed;
                }));
        }

    private:

        class EmitGuard
        {
        public:

            explicit EmitGuard(const Source& signal) :
                _signal(signal)
            {
                _signal._emitting = true;
            }

            ~EmitGuard()
            {
                _signal._emitting = false;
                _signal.compact();
            }

        private:

            const Source& _signal;
        };

        void eraseRecord(container_type::iterator it)
        {
            auto last = _slots.end();
            --last;

            if (it != last)
            {
                *it = std::move(*last);
            }

            _slots.pop_back();
        }

        void compact() const
        {
            std::erase_if(_slots,
                [](const SlotRecord& record)
                {
                    return record.removed;
                });
        }

        mutable bool _emitting = false;
        mutable container_type _slots;
    };
}
