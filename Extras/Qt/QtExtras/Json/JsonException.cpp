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

    separator sep(_T("->"));

    for (auto& info : m_path | std::views::reverse)
    {
        f << sep << "[" << info.key << "] (" << TypeToString(info.jsonType) << "/" << info.cppType << ")";
    }

    f << ". Message: '" << theMessage << "'";
    
    return f.str();
}

JsonException::JsonException(String message, ValueInfo info) : GeneralException(std::move(message))
{
    m_path.push_back(std::move(info));
}

JsonException::JsonException(JsonException& inner, ValueInfo info) :
    GeneralException(std::move(inner.What())),
    m_path(std::move(inner.m_path))
{
    m_path.push_back(std::move(info));
}

void JsonException::append(ValueInfo info)
{
    m_path.push_back(std::move(info));
}
