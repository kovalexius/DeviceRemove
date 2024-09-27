///////////////////////////////////////////////////////////////////////////////////////
///
///	@file Str.h
/// 
/// @brief Прототипы функций для работы со строками, в т.ч. осуществляющих преобразование из
/// широкой строки std::wstring в узкую std::string и наоборот
/// 
/// @date	23.06.2023
/// @author Ковалев А.Г., Зимин Н.В. (рефакторинг)
///
///////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#endif
namespace sudis::base
{
	/// преобразование utf-8 -> utf-16
	/// Перегрузится под windows.
	/// Под linux вызовется std::string реализация
	std::wstring s_ws(const char* str);

	/// @brief Преобразует узкую строку std::string в широкую std::wstring
	/// из utf8 в utf16 соответственно
	inline std::wstring s_ws(const std::string& str) { return s_ws(str.c_str()); }
#ifdef __linux__
    std::wstring s_wsCP(const char* str);
#endif
    std::string ws_s1251(const wchar_t* wstr);
	inline std::wstring s_ws1251(const std::string& str)
	{
#ifdef __linux__
		return s_ws(str);
#else
		int sz_src = (int)strlen(str.c_str());
		if (sz_src == 0)
			return {};
		auto sz = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), sz_src, nullptr, 0);
		if (sz <= 0)
			return {};

		std::wstring w_str(sz, L'\0');
		sz = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), sz_src, (LPWSTR)w_str.data(), sz);
		if (sz <= 0)
			return {};

		return w_str;
#endif
	}


	/// преобразование utf-16 -> utf-8
	/// Перегрузится под windows. 
	/// Под linux вызовется std::wstring реализация
	std::string ws_s(const wchar_t* wstr);
	/// @brief Преобразует широкую строку std::wstring в узкую std::string
	/// из utf16 в utf8 соответственно
	inline std::string ws_s(const std::wstring& wstr) { return ws_s(wstr.c_str()); }
	/// @brief Разбивает строку на подстроки
	/// @param [in] _inStr входная строка
	/// @param [in] _delimiter разделитель, по которому будет биться строка
	/// @return массив подстрок
	std::vector<std::string> splitString(const std::string& _inStr, const std::string& _delimiter);
}




