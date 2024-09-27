//////////////////////////////////////////////////////////////////////////////////////////////
///
/// @brief Шаблонный класс автоматической очистки ресурсов.
/// Использование: Написать делетер с оператором operator()(), через аргументы шаблона задать 
/// тип ресурса и его делетер, например так:
/// struct SomeHandleDeleter
/// {
///		void operator()(HANDLE& _handle) {
///			someClose(_handle);
///		}
///	};
///	typedef RaiiWrapper<HANDLE, SomeHandleDeleter> SomeHandle;
/// 
/// @date: 21.12.2023
/// @author akovalev@phoenixit.ru
/// 
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <functional>

namespace sudis::base
{
	template<typename T, typename DELETER>
	class RaiiWrapper
	{
	public:
		/// @brief Пустой конструктор, если мы не можем сразу передать значение хендла
		RaiiWrapper() : m_deleter(DELETER()), m_handle(NULL)
		{}

		/// @brief Конструктор принимающий значение хендла.
		RaiiWrapper(T _handle) : m_deleter(DELETER()), m_handle(_handle)
		{}

		~RaiiWrapper()
		{
			m_deleter(m_handle);
		}

		/// @brief Прочитать или поменять значение хендла по ссылке
		T& getRef()
		{
			return m_handle;
		}

		/// @brief Вернуть значение хендла
		T get() const
		{
			return m_handle;
		}

		/// @brief Прочитать или поменять значение хендла по указателю
		T* getPtr()
		{
			return &m_handle;
		}

	private:
		/// @brief Делетер который задается при инстанцировании шаблона
		std::function<void(T&)> m_deleter;
		
		/// @brief Хендл произвольного типа, т.к. тип шаблонный
		T m_handle;
	};
}

#ifdef _WIN32
#include <windows.h>
#include <winsvc.h>
#include <SetupAPI.h>

namespace sudis::base
{
	struct ScHandleDeleter
	{
		void operator()(SC_HANDLE& _handle);
	};
	using ScHandle = RaiiWrapper<SC_HANDLE, ScHandleDeleter>;

	struct HDevInfoDeleter
	{
		void operator()(HDEVINFO& _handle);
	};
	using HDevInfo = RaiiWrapper<HDEVINFO, HDevInfoDeleter>;
}

#else
namespace sudis::base
{
struct FdHandleDeleter
{
	void operator()(int _fd);
};
using FdHandle = RaiiWrapper<int, FdHandleDeleter>;
}
#endif
