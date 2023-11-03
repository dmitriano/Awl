/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/StaticChain.h"
#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

#include <type_traits>

namespace awl
{
    AWL_DEFINE_EXCEPTION(FactoryException)

    template <class T>
    using FactoryFuncPtr = std::add_pointer_t<T()>;

    template <class T>
    T create(const awl::Char* name)
    {
        using FuncPtr = awl::FactoryFuncPtr<T>;

        auto& chain = static_chain<FuncPtr>();
        
        auto i = chain.find(name);

        if (i == chain.end())
        {
            throw FactoryException(format() << "Factory function '" << name << "' not found.");
        }

        FuncPtr func = i->value();

        return func();
    }
}

// Parameterless factory.
#define AWL_FACTORY_FUNC_NAME(name) name##_FactoryFunc
#define AWL_FACTORY_FUNC_SIGNATURE(T, name) T AWL_FACTORY_FUNC_NAME(name)()

#define AWL_FACTORY(T, name) \
    static AWL_FACTORY_FUNC_SIGNATURE(T, name); \
    static awl::StaticLink<awl::FactoryFuncPtr<T>> name##_FactoryLink(_T(#name), &AWL_FACTORY_FUNC_NAME(name)); \
    static AWL_FACTORY_FUNC_SIGNATURE(T, name)

#define AWT_DISABLED_FACTORY(T, name) \
    static AWL_FACTORY_FUNC_SIGNATURE(T, name)
