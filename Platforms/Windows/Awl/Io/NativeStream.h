/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

        WinStream(FileHandle&& h) : _hFile(std::forward<FileHandle>(h))
        {}

        bool operator == (const WinStream& other) const
        {
            return _hFile == other._hFile;
        }

        bool operator != (const WinStream& other) const
        {
            return !operator == (other);
        }

        size_t length() const override
        {
            return static_cast<size_t>(fileSizeHelper());
        }

        size_t position() const override
        {
            return static_cast<size_t>(filePointerHelper());
        }

        size_t read(uint8_t* buffer, size_t count) override
        {
            const DWORD nNumberOfBytesToRead = static_cast<DWORD>(count);
            assert(nNumberOfBytesToRead == count);
            DWORD NumberOfBytesRead = 0;

            check(::ReadFile(_hFile, buffer, nNumberOfBytesToRead, &NumberOfBytesRead, NULL) != FALSE);

            return NumberOfBytesRead;
        }

        void write(const uint8_t* buffer, size_t count) override
        {
            const DWORD nNumberOfBytesToWrite = static_cast<DWORD>(count);
            assert(nNumberOfBytesToWrite == count);
            DWORD NumberOfBytesWritten = 0;

            if (::WriteFile(_hFile, buffer, nNumberOfBytesToWrite, &NumberOfBytesWritten, NULL) == FALSE)
            {
                throw Win32Exception(_T("::WriteFile failed. This may indicate that the disk is full. Win32 Error: "));
            }

            if (nNumberOfBytesToWrite != NumberOfBytesWritten)
            {
                throw IoError(std::format(_T("Requested {} bytes, but actually written {}."), nNumberOfBytesToWrite, NumberOfBytesWritten));
            }
        }

        bool end() override
        {
            return fileSizeHelper() == filePointerHelper();
        }

        void seek(std::size_t pos, bool begin = true) override
        {
            LARGE_INTEGER li;

            li.QuadPart = pos;

            check(::SetFilePointerEx(_hFile, li, NULL, begin ? FILE_BEGIN : FILE_END) != INVALID_SET_FILE_POINTER);
        }

        void move(std::ptrdiff_t offset) override
        {
            LARGE_INTEGER li;

            li.QuadPart = offset;

            check(::SetFilePointerEx(_hFile, li, NULL, FILE_CURRENT) != INVALID_SET_FILE_POINTER);
        }

        void flush() override
        {
            check(::FlushFileBuffers(_hFile) != FALSE);
        }

        void truncate() override
        {
            check(::SetEndOfFile(_hFile) != FALSE);
        }

        String fileName() const
        {
            TCHAR buf[MAX_PATH];
            // It is not clear what is the difference with FILE_NAME_NORMALIZED.
            const DWORD len = GetFinalPathNameByHandle(_hFile, buf, MAX_PATH, FILE_NAME_OPENED);

            if (len > MAX_PATH)
            {
                return _T("<Too long path>");
            }

            return buf;
        }

    private:

        static void check(bool success)
        {
            if (!success)
            {
                throw Win32Exception();
            }
        }
        
        LONGLONG fileSizeHelper() const
        {
            LARGE_INTEGER li;

            li.QuadPart = 0;

            check(::GetFileSizeEx(_hFile, &li) != FALSE);

            return li.QuadPart;
        }

        LONGLONG filePointerHelper() const
        {
            LARGE_INTEGER liOfs = { 0 };
            LARGE_INTEGER liNew = { 0 };

            check(::SetFilePointerEx(_hFile, liOfs, &liNew, FILE_CURRENT) != INVALID_SET_FILE_POINTER);

            return liNew.QuadPart;
        }

        FileHandle _hFile;
    };

    using UniqueStream = WinStream<UniqueFileHandle>;
    
    using SharedStream = WinStream<SharedFileHandle>;

    inline UniqueFileHandle createUniqueFile(const String& file_name)
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
            throw Win32Exception(std::format(_T("Cannot open file '{}' for updating"), file_name));
        }

        return hFile;
    }

    inline bool openedExisting()
    {
        return ::GetLastError() == ERROR_ALREADY_EXISTS;
    }

    inline UniqueFileHandle openUniqueFile(const String& file_name)
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
            throw Win32Exception(std::format(_T("Cannot open file '{}' for reading."), file_name));
        }

        return hFile;
    }
}
