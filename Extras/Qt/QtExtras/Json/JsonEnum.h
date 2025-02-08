#pragma once

#include "Qtil/Json/JsonSerializer.h"

#include "Awl/EnumTraits.h"

namespace qtil
{
    template <class T>
    class JsonSerializer<T, std::enable_if_t<std::is_enum_v<T>>>
    {
    private:

        using U = std::underlying_type_t<T>;

    public:

        using value_type = T;

        void FromJson(const QJsonValue& jv, value_type& val)
        {
            JsonSerializer<std::string> formatter;

            std::string str_val;
            formatter.FromJson(jv, str_val);

            auto& names = awl::EnumTraits<T>::names();

            const auto i = names.find(str_val);

            if (i == names.end())
            {
                throw JsonException(awl::format() << _T("Wrong enum value '" << awl::FromAString(str_val) << "'."));
            }

            const size_t index = i - names.begin();

            val = static_cast<T>(index);
        }

        void ToJson(const value_type & val, QJsonValue & jv)
        {
            JsonSerializer<std::string> formatter;

            auto& names = awl::EnumTraits<T>::names();

            const size_t index = static_cast<size_t>(val);

            formatter.ToJson(names[index], jv);
        }
    };
}
