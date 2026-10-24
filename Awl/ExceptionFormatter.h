/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

#include <concepts>
#include <exception>
#include <format>

namespace awl
{
    // Borrows the exception; retain it until formatting has completed.
    class ExceptionView
    {
    public:

        explicit ExceptionView(const std::exception& error) noexcept : _error(error)
        {}

        const std::exception& exception() const noexcept { return _error; }

    private:

        const std::exception& _error;
    };

    [[nodiscard]] inline ExceptionView formatException(const std::exception& error) noexcept
    {
        return ExceptionView(error);
    }
}

namespace std
{
    template <std::derived_from<awl::Exception> T, class CharT>
    struct formatter<T, CharT> : formatter<awl::String, CharT>
    {
        template <class FormatContext>
        auto format(const T& error, FormatContext& ctx) const
        {
            return formatter<awl::String, CharT>::format(error.message(), ctx);
        }
    };

    template <class CharT>
    struct formatter<awl::ExceptionView, CharT> : formatter<awl::String, CharT>
    {
        template <class FormatContext>
        auto format(const awl::ExceptionView& view, FormatContext& ctx) const
        {
            const std::exception& error = view.exception();

            if (const auto* awl_error = dynamic_cast<const awl::Exception*>(&error))
            {
                return formatter<awl::String, CharT>::format(awl_error->message(), ctx);
            }
            else
            {
                return formatter<awl::String, CharT>::format(awl::fromACString(error.what()), ctx);
            }
        }
    };
}
