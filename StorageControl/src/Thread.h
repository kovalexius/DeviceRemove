///////////////////////////////////////////////////////////////////////////////
///
///	\file	Thread.h
///
///	\brief	Шаблонный базовый класс для создания потока,
///
///	\date	17.07.2023
///	\author	nzimin@phoenixit.ru
///
///////////////////////////////////////////////////////////////////////////////
#ifndef SUDIS_BASE_THREAD_H_INCLUDED
#define SUDIS_BASE_THREAD_H_INCLUDED

#include <thread>
#include <atomic>
#include <Event.h>

namespace sudis
{
namespace base
{

///////////////////////////////////////////////////////////////////////////
/// \class Thread
/// \brief класс потока, выполняющего циклические действия
template<class T>
class Thread
{
	/// данные класса

    std::thread					m_thread;       ///< c++ объект-поток
    std::atomic<bool>           m_stopFlag;     ///< флаг завершения работы
    Event						m_wakener;      ///< событие "пробуждения"

public:

    /// \brief конструктор, при создании поток не запускается
    Thread()
    : m_stopFlag(false)
    {}

	/// \brief деструктор, если поток активен - завершение работы
    virtual ~Thread()
	{
        stop(false);
	}

    /// \brief признак того, что поток работает
    bool isActive() const
    {
        return m_thread.joinable() && !m_stopFlag.load();
    }

    /// \brief Начать выполнение
    /// \return false если поток не может быть запущен
    bool start()
	{
        // поток уже запущен
        if(m_thread.joinable())
            return true;

		// Запуск нового потока
		try {
            m_thread = std::thread(&Thread<T>::threadBody, this);
        } catch (std::exception& /* ex */) {
            return false;
		}

		return true;
	}

    /// \brief "разбудить" поток, который находится в режиме ожидания между циклами
    /// \return false если поток не запущен
    bool wake()
    {
        if(!m_thread.joinable())
            return false;

        m_wakener.set();
        return true;
    }

	/// \brief Остановка потока
	/// \param [in] async асинхронное или синхронное выполнение
	///		(ждать остановки или нет)
    bool stop(bool async)
	{
        m_stopFlag.store(true);

		// уже неактивен
		if (!m_thread.joinable())
			return true;

        // разбудить
        m_wakener.set();

		// при асинхронном выполнении или при вызове из самого потока
		//  выход
		if (async || (std::this_thread::get_id() == m_thread.get_id()))
			return true;

		try {
			m_thread.join();
		}
		catch (std::exception& /* ex */) {
            return false;
		}
		return true;
	}

private:

    /// \breaf нужно ли закончить выполнение
    bool isStopping() { return m_stopFlag.load(); }

    /// \breaf состояние простоя после выполненного цикла
    void  idle() { m_wakener.wait(getSleepTimeout()); }

    /// \breaf выполняется в потоке перед началом основного цикла
    virtual void init() = 0;

    /// \breaf выполняется в потоке после завершения основного цикла
    virtual void unInit() = 0;

    /// \breaf время ожидания между циклами
    virtual uint32_t getSleepTimeout() const = 0;

    /// \breaf основной цикл выполняемых операций
    /// \return false, если необходимо сразу начинать следующий цикл, иначе - после таймаута
    virtual bool run() = 0;

    ///\brief "тело" потока
    void threadBody()
	{
        // выполнение начато...
        init();

		// пока не остановлено...
        while (!isStopping()) {

            // непосредственное выполнение цикла операций
            if(!run()) {
                // если run() возвращает false, то без ожидания к новому циклу
                continue;
            }
            idle();
		}

		// завершение...
        unInit();
	}
};

} // namespace base
} // namespace sudis

#endif // SUDIS_BASE_THREAD_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
// End of Thread.h
///////////////////////////////////////////////////////////////////////////////
