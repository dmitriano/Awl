/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"

namespace awl::testing
{
    template <class T>
    class AttributeProvider
    {
    public:

        virtual bool TryGet(const String& name, T& val) = 0;
    };

    template <class T, class ProviderImpl>
    class ProviderAdapter : public AttributeProvider<T>
    {
    public:

        ProviderAdapter(ProviderImpl& impl) : m_impl(impl) {}

        bool TryGet(const String& name, T& val) override
        {
            return m_impl.TryGet(name, val);
        }

    private:

        ProviderImpl& m_impl;
    };
}
