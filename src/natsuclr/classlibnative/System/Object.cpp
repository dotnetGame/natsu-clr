//
// Natsu CLR Class Lib Native
//
#include <classlibnative/System/Object.hpp>
#include <cassert>

using namespace clr;
using namespace clr::corlib;

ObjectHeader& Object::GetHeader() noexcept
{
    return *reinterpret_cast<ObjectHeader*>(uintptr_t(this) - sizeof(ObjectHeader));
}

uintptr_t Object::GetType(ObjectRef<> this_)
{
    assert(false);
    return 0;
}
