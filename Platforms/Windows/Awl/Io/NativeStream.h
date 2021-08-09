#pragma once

#include "Awl/Io/Handle.h"

#include "Awl/StringFormat.h"
#include "Awl/Io/Stream.h"
#include "Awl/Io/IoException.h"
#include "Awl/Io/NativeException.h"

namespace awl::io
{
    template <class FileHandle>
    class WinStream : public IoStream
    {
    public:

        WinStream() = default;

        WinStream(FileHandle&& h) : m_hFile(std::forward<FileHandle>(h))
        {
        }

        size_t GetLength() override
        {
            return static_cast<size_t>(GetFileSizeHelper());
        }

        size_t GetPosition() override
        {
            return static_cast<size_t>(GetFilePointerHelper());
        }

        size_t Read(uint8_t* buffer, size_t count) override
        {
            const DWORD nNumberOfBytesToRead = static_cast<DWORD>(count);
            assert(nNumberOfBytesToRead == count);
            DWORD NumberOfBytesRead = 0;

            Check(::ReadFile(m_hFile, buffer, nNumberOfBytesToRead, &NumberOfBytesRead, NULL) != FALSE);

            return NumberOfBytesRead;
        }

        void Write(const uint8_t* buffer, size_t count) override
        {
            const DWORD nNumberOfBytesToWrite = static_cast<DWORD>(count);
            assert(nNumberOfBytesToWrite == count);
            DWORD NumberOfBytesWritten = 0;

            if (::WriteFile(m_hFile, buffer, nNumberOfBytesToWrite, &NumberOfBytesWritten, NULL) == FALSE)
            {
                throw IoError(format()
                    << _T("::WriteFile failed. This may indicate that the disk is full. Win32 Error: ")
                    << ::GetLastError());
            }

            if (nNumberOfBytesToWrite != NumberOfBytesWritten)
            {
                throw IoError(format() << _T("Requested ") << nNumberOfBytesToWrite
                    << _T(" bytes, but actually written ") << NumberOfBytesWritten << _T("."));
            }
        }

        bool End() override
        {
            return GetFileSizeHelper() == GetFilePointerHelper();
        }

        void Seek(std::size_t pos, bool begin = true) override
        {
            LARGE_INTEGER li;

            li.QuadPart = pos;

            Check(::SetFilePointerEx(m_hFile, li, NULL, begin ? FILE_BEGIN : FILE_END) != INVALID_SET_FILE_POINTER);
        }

        void Move(std::ptrdiff_t offset) override
        {
            LARGE_INTEGER li;

            li.QuadPart = offset;

            Check(::SetFilePointerEx(m_hFile, li, NULL, FILE_CURRENT) != INVALID_SET_FILE_POINTER);
        }

        void Flush() override
        {
            Check(::FlushFileBuffers(m_hFile) != FALSE);
        }

        void Truncate() override
        {
            Check(::SetEndOfFile(m_hFile) != FALSE);
        }

    private:

        void Check(bool success)
        {
            if (!success)
            {
                throw Win32Exception();
            }
        }
        
        LONGLONG GetFileSizeHelper()
        {
            LARGE_INTEGER li;

            li.QuadPart = 0;

            Check(::GetFileSizeEx(m_hFile, &li) != FALSE);

            return li.QuadPart;
        }

        LONGLONG GetFilePointerHelper()
        {
            LARGE_INTEGER liOfs = { 0 };
            LARGE_INTEGER liNew = { 0 };

            Check(::SetFilePointerEx(m_hFile, liOfs, &liNew, FILE_CURRENT) != INVALID_SET_FILE_POINTER);

            return liNew.QuadPart;
        }

        FileHandle m_hFile;
    };

    using UniqueStream = WinStream<UniqueFileHandle>;
    
    using SharedStream = WinStream<SharedFileHandle>;

    inline UniqueFileHandle CreateUniqueFile(const String& file_name)
    {
        //CREATEFILE2_EXTENDED_PARAMETERS extendedParams = { 0 };
        //extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
        //extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
        //extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
        //extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
        //extendedParams.lpSecurityAttributes = nullptr;
        //extendedParams.hTemplateFile = nullptr;

        HANDLE hFile = ::CreateFile2(
            file_name.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0, //FILE_SHARE_READ | FILE_SHARE_WRITE,
            OPEN_ALWAYS,
            NULL //&extendedParams
        );

        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD dw_err = ::GetLastError();

            throw IoError(format() << _T("Cannot open file ')" << file_name << "' for updating, error = " << dw_err));
        }

        return hFile;
    }

    inline UniqueFileHandle OpenUniqueFile(const String& file_name)
    {
        //CREATEFILE2_EXTENDED_PARAMETERS extendedParams = { 0 };
        //extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
        //extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
        //extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
        //extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
        //extendedParams.lpSecurityAttributes = nullptr;
        //extendedParams.hTemplateFile = nullptr;

        HANDLE hFile = ::CreateFile2(
            file_name.c_str(),
            GENERIC_READ,
            0, //FILE_SHARE_READ | FILE_SHARE_WRITE,
            OPEN_EXISTING,
            NULL //&extendedParams
        );

        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD dw_err = ::GetLastError();

            throw IoError(format() << _T("Cannot open file ')" << file_name << "' for reading, error = " << dw_err));
        }

        return hFile;
    }
}
