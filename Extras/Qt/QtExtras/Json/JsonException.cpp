#include "QtExtras/Json/JsonException.h"
#include "QtExtras/Json/JsonHelpers.h"

#include "Awl/StringFormat.h"
#include "Awl/Separator.h"

#include <ranges>

using namespace awl;

String JsonException::What() const
{
    //format f;
    ostringstream f;

    f << "Path: ";

    {
        separator sep(_T("->"));

        for (const std::string& key : m_path | std::views::reverse | std::views::transform(std::mem_fn(&ValueInfo::key)))
        {
            f << sep << key;
        }
    }

    f << "\n";

    f << "Message: '" << theMessage << "'";

    f << "\n";

    f << "Details: ";

    f << "\n";

    separator sep(_T("\n"));

    for (auto& info : m_path | std::views::reverse)
    {
        f << sep << "[" << info.key << "] (" << TypeToString(info.jsonType) << "/" << info.cppType << ")";
    }

    return f.str();
}

JsonException::JsonException(String message, ValueInfo info) : GeneralException(std::move(message))
{
    m_path.push_back(std::move(info));
}

void JsonException::append(ValueInfo info)
{
    m_path.push_back(std::move(info));
}
