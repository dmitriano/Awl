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

    template <class T, typename... Args>
    using FactoryFuncPtr = std::add_pointer_t<T(Args... args)>;

    template <class T, typename... Args>
    T create(const char* name, Args&&... args)
    {
        using FuncPtr = awl::FactoryFuncPtr<T, Args...>;

        auto& chain = static_chain<FuncPtr>();
        
        auto i = chain.find(name);

        if (i == chain.end())
        {
            throw FactoryException(format() << "Factory function '" << name << "' not found.");
        }

        FuncPtr func = i->value();

        return func(std::forward<Args>(args)...);
    }

    template <typename Func>
    struct factory_traits;

    template <class T, typename... Args>
    struct factory_traits<T(Args...)>
    {
        using Link = awl::StaticLink<awl::FactoryFuncPtr<T, Args...>>;
    };
}

// Factory with parameters (signature is unknown).
#define AWL_FACTORY_FUNC_NAME(name) name##_FactoryFunc
#define AWL_REGISTER_FACTORY(func, name) \
    static typename awl::factory_traits<decltype(func)>::Link name##_FactoryLink(#name, &func);

// Parameterless factory.
#define AWL_FACTORY_FUNC_SIGNATURE(T, name) T AWL_FACTORY_FUNC_NAME(name)()
#define AWL_FACTORY(T, name) \
    static AWL_FACTORY_FUNC_SIGNATURE(T, name); \
    static awl::StaticLink<awl::FactoryFuncPtr<T>> name##_FactoryLink(#name, &AWL_FACTORY_FUNC_NAME(name)); \
    static AWL_FACTORY_FUNC_SIGNATURE(T, name)

#define AWL_DISABLED_FACTORY(T, name) \
    static AWL_FACTORY_FUNC_SIGNATURE(T, name)
