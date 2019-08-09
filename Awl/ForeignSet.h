#pragma once

#include "Awl/ObservableSet.h"

#include <assert.h>

namespace awl
{
    template <class ForeignKeyGetter, class T, class PrimaryKeyGetter>
    class foreign_set : public Observer<INotifySetChanged<T>>
    {
    private:

        using ForeignKey = typename function_traits<ForeignKeyGetter>::result_type;

        class Multi
        {
        public:

            ForeignKey GetForeignKey() const
            {
                assert(!m_set.empty());
                return foreignKeyGetter(*m_set.front());
            }

            observable_set<const T *, KeyCompare<const T *, PrimaryKeyGetter>> m_set;

        private:

            ForeignKeyGetter foreignKeyGetter;
        };

    public:

        void OnAdded(const T & val) override
        {
            auto i = m_set.find(foreignKeyGetter(val));

            if (i != m_set.end())
            {
                Multi & m = *i;
                m.m_set.insert(&val);
            }
            else
            {
                Multi m;
                m.m_set.insert(&val);
                m_set.insert(std::move(m));
            }
        }

        void OnRemoving(const T & val) override
        {
            auto i = m_set.find(foreignKeyGetter(val));

            assert(i != m_set.end());

            Multi & m = *i;

            if (m.m_set.size() == 1)
            {
                m_set.erase(m);
            }
        }

        void OnClearing() override
        {
            m_set.clear();
        }

        observable_set<Multi, FuncCompare<Multi, ForeignKey, &Multi::GetForeignKey>> m_set;

    private:

        ForeignKeyGetter foreignKeyGetter;
    };
}
