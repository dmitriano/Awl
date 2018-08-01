#pragma once

#include "Awl/Testing/TestMap.h"

#include <exception>
#include <functional>
#include <list>
#include <string>
#include <sstream>

namespace UnitTesting
{
    typedef awl::String TString;

    //! The basic exception class for Lines Game engine.
    class TestException : public std::exception
    {
    private:

        const TString theMessage;

    public:

        explicit TestException() : theMessage(_T("No messsage provided."))
        {
        }

        explicit TestException(const TString & message) : theMessage(message)
        {
        }

        const TString & GetMessage() const
        {
            return theMessage;
        }

        virtual const char * what() const throw() override;
    };

    class Assert
    {
    public:

        static void IsTrue(bool val, const TCHAR * message = nullptr)
        {
            if (!val)
            {
                if (message != nullptr)
                {
                    throw TestException(message);
                }
                else
                {
                    throw TestException(_T("The value is not true."));
                }
            }
        }

        static void IsFalse(bool val, const TCHAR * message = nullptr)
        {
            IsTrue(!val, message);
        }

        template <typename T>
        static void AreEqual(T left, T right, const TCHAR * message = nullptr)
        {
            if (left != right)
            {
                std::ostringstream out;

                out << _T("Actual ") << left << _T(" expected ") << right;

                if (message != nullptr)
                {
                    out << message;
                }

                throw TestException(out.str());
            }
        }
    };
}
