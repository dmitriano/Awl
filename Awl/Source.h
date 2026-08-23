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
        private ISignal<Args...>
    {
    public:

        using Signal = ISignal<Args...>;

    private:

        using Slot = typename Signal::Slot;

        struct SlotRecord
        {
            Slot slot;
            bool removed = false;
        };

    public:

        using container_type = std::vector<SlotRecord>;

        Signal& signal() noexcept
        {
            return *this;
        }

        template<typename ...Params>
        void emit(const Params&... args)
            requires (std::invocable<Slot&, const Params&...>)
        {
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

            if (isEmitting())
            {
                it->removed = true;
            }
            else
            {
                eraseRecord(it);
            }

            return true;
        }

        class EmitGuard
        {
        public:

            explicit EmitGuard(Source& signal) :
                _source(signal)
            {
                ++_source._emissionDepth;
            }

            ~EmitGuard()
            {
                assert(_source._emissionDepth != 0);

                --_source._emissionDepth;

                if (!_source.isEmitting())
                {
                    _source.compact();
                }
            }

        private:

            Source& _source;
        };

        bool isEmitting() const noexcept
        {
            return _emissionDepth != 0;
        }

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

        void compact()
        {
            std::erase_if(_slots,
                [](const SlotRecord& record)
                {
                    return record.removed;
                });
        }

        std::size_t _emissionDepth = 0;
        container_type _slots;
    };
}
