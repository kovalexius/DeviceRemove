///////////////////////////////////////////////////////////////////////////////////////
///
///	@file StrWindows.cpp
///
/// @brief Реализация функций преобразования строк узкая-широкая и наоборот под Windows
///
/// @date	23.06.2023
/// @author Ковалев А.Г.
///
///////////////////////////////////////////////////////////////////////////////////////

//#ifdef _WIN32

#include <vector>
#ifdef UNIX
#include <CSP_WinDef.h>
#endif
#include "Str.h"
#include <string.h>

namespace sudis::base
{
    std::wstring s_ws(const char* str)
    {
        int sz_src = (int)strlen(str);
        if (sz_src == 0)
            return {};
        
        auto sz = ::MultiByteToWideChar(CP_UTF8, 0, str, sz_src, nullptr, 0);
        if (sz <= 0)
            return {};

        std::wstring w_str(sz, L'\0');

        sz = ::MultiByteToWideChar(CP_UTF8, 0, str, sz_src, (LPWSTR)w_str.data(), sz);
        if (sz <= 0)
            return {};

        return w_str;
    }

    std::string ws_s(const wchar_t* wstr)
    {
        int sz_src = (int)wcslen(wstr);
        if (sz_src == 0)
            return {};

        auto sz = ::WideCharToMultiByte(CP_UTF8, 0, wstr, sz_src, nullptr, 0, nullptr, nullptr);
        if (sz <= 0) 
            return {};
        
        std::string str(sz, L'\0');

        sz = ::WideCharToMultiByte(CP_UTF8, 0, wstr, sz_src, (LPSTR)str.data(), sz, nullptr, nullptr);
        if (sz <= 0)
            return {};
        
        return str;
    }
}

//#endif
