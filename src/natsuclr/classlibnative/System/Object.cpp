//
// Natsu CLR Class Lib Native
//
#include <classlibnative/System/Object.hpp>

using namespace clr;
using namespace clr::corlib;

ObjectHeader & Object::GetHeader() noexcept
{
	return *reinterpret_cast<ObjectHeader*>(uintptr_t(this) - sizeof(ObjectHeader));
}
