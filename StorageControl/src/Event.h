///////////////////////////////////////////////////////////////////////////////
///
///	\file		Event.h
///
///	\brief		Определение класса "событие", для синхронизации потоков
///
///	\date		09.09.2022
///	\authors	nzimin@phoenixit.ru
///
///////////////////////////////////////////////////////////////////////////////
#ifndef SUDIS_BASE_MT_EVENT_H_INCLUDED
#define SUDIS_BASE_MT_EVENT_H_INCLUDED

#include <condition_variable>
#include <mutex>
#include <chrono>

namespace sudis
{
namespace base
{
	///////////////////////////////////////////////////////////////////////////
	/// \class Event
	/// \brief объект-событие для синхронизации потоков
	class Event
	{
		/// данные класса

		std::mutex				m_mtx;		///< для блокировки выполнения кода из разных потоков
		std::condition_variable	m_cv;		///< для ожидания сигнала
		bool					m_signal;	///< флаг пришедшего сигнала

	public:

		/// \brief конструктор,
		///  	создаваемое событие всегда "выключено"
		Event()
		: m_signal(false)
		{}

		/// \brief деструктор
		~Event()
		{}

		/// \brief ожидание события
		/// \param [in] timeout интервал ожидания в миллисекундах, если равен \e UINT32_MAX
		///			то ждать всегда
		/// \return false,если в течение заданного интервала событие не произошло
		bool wait(uint32_t timeout = UINT32_MAX)
		{
			std::unique_lock<std::mutex> lock(m_mtx);

			if (timeout == UINT32_MAX) {
				while(!m_signal)
					m_cv.wait(lock);
				m_signal = false;
				return true;
			}

			auto now = std::chrono::system_clock::now();
			while (!m_signal) {
				if (m_cv.wait_until(lock, now + std::chrono::milliseconds(timeout)) == std::cv_status::timeout) {
					m_signal = false;
					return false;
				}
			}
			m_signal = false;
			return true;
		}

		/// \brief проверка, произошло ли событие (без ожидания)
		/// \return false,если событие не произошло
		bool check()
		{
			std::unique_lock<std::mutex> lock(m_mtx);

			while (!m_signal) {
				if (m_cv.wait_for(lock, std::chrono::milliseconds(0)) == std::cv_status::timeout) {
					m_signal = false;
					return false;
				}
			}

			m_signal = false;
			return true;
		}

		/// \brief установить событие, "включить"
		/// \param _all если задано true, то оповестить все потоки, 
		/// иначе только один поток получает оповещение
		void set(bool _all = false)
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			m_signal = true;
			if(_all)
				m_cv.notify_all();
			else
				m_cv.notify_one();
		}

		/// \brief сбросить событие
		void reset()
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			m_signal = false;
		}
	};

} // namespace base
} // namespace sudis

#endif	// SUDIS_BASE_MT_EVENT_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
// End of Event.h
///////////////////////////////////////////////////////////////////////////////
