#include "RaiiWrapper.h"

/// Виндовые реализации
#ifdef _WIN32

namespace sudis::base
{
	void ScHandleDeleter::operator()(SC_HANDLE& _handle)
	{
		if (_handle)
			CloseServiceHandle(_handle);
	}

	void HDevInfoDeleter::operator()(HDEVINFO& _handle)
	{
		SetupDiDestroyDeviceInfoList(_handle);
	}
}

#else
namespace sudis::base
{
void FdHandleDeleter::operator()(int _fd)
{
	if(_fd >= 0)
		close(_fd);
}
}
#endif
