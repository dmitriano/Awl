#pragma once

#include "Qtil/DecimalType.h"
#include "Qtil/StringConversion.h"

#include "Awl/EnumTraits.h"
#include "Awl/Crypto/IntHash.h"
#include "Awl/Crypto/FixedHash.h"

#include <QHash>
#include <QString>
#include <QStringList>
#include <QObject>

#include <functional>
#include <atomic>
#include <sstream>
#include <ranges>
#include <functional>
#include <concepts>

namespace qtil
{
    using FixedHash = awl::crypto::FixedHash<awl::crypto::Int64Hash>;

    template <class Hash>
    FixedHash::value_type hashString(const Hash& hash, const QString& s)
    {
        const auto range = s | std::views::transform(std::mem_fn(&QChar::toLatin1));

        return hash(range.begin(), range.end());
    }

    void removeTrailingZeros(QString & s);

    inline QString toString(const qtil::decimal& d)
    {
        std::ostringstream out;

        out << d;

        return QString::fromStdString(out.str());
    }

    template <class Enum, size_t N>
    const char* enumToStringPlain(Enum val, const char* (&names)[N])
    {
        const size_t index = static_cast<size_t>(val);

        if (index >= N)
        {
            return "Wrong enum value";
        }

        return names[index];
    }

    // Conversion enum to QString

    template <class Enum>
    QString enumToString(Enum val)
    {
        return QString::fromStdString(awl::enum_to_string(val));
    }

    template <class Enum>
    QString enumToString(const std::atomic<Enum>& atomic_val)
    {
        return enumToString(atomic_val.load());
    }

    template <class Enum>
    Enum enumFromString(const QString& name)
    {
        return awl::enum_from_string<Enum>(name.toStdString());
    }

    // Can be used with QCoreApplication::translate("main", key).
    template <class Enum, class Convert, class... Args>
    QStringList convertEnumToStringList(Convert&& conv, Args&&... args)
    {
        QStringList list;

        auto& names = awl::EnumTraits<Enum>::names();

        for (const auto& name : names)
        {
            list.append(std::invoke(std::forward<Convert>(conv), std::forward<Args>(args)..., name));
        }

        return list;
    }

    template <class Enum>
    QStringList enumToStringList()
    {
        return convertEnumToStringList<Enum>(QString::fromStdString);
    }

    // QObject::tr is not a virtual function, it requires the derived object type.
    template <class Enum, class Object>
    QStringList enumToStringListTr(const Object* p_object)
    {
        auto convert = [&p_object](const std::string& name) -> QString
        {
            // tr() has additional default arguments.
            return p_object->tr(name.c_str());
        };

        return convertEnumToStringList<Enum>(convert);
    }
}
