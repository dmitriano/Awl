#pragma once

#include <exception>
#include <functional>
#include <list>
#include <string>
#include <sstream>

#if !defined(_T)	
	#define _T(quoted_string) quoted_string
#endif

namespace UnitTesting
{
	typedef char TCHAR;

	typedef std::basic_string<TCHAR> TString;
	
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

		template <typename T>
		static void AreEqual(T left, T right, const TCHAR * message = nullptr)
		{
			if (left != right)
			{
				std::ostringstream out;

				out << _T("Actual ") << left << _T(" provided ") << right;

				if (message != nullptr)
				{
					out << message;
				}

				throw TestException(out.str());
			}
		}
	};

	class Test
	{
	public:

		void Run()
		{
			testFunc();
		}

	private:

		std::string testName;

		std::function<void()> testFunc;
	};
}
