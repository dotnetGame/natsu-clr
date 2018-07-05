//
// Natsu CLR VM
//
#include <vm/Thread.hpp>
#include <vm/Interpreter.hpp>
#include <utils.hpp>

using namespace clr;
using namespace clr::vm;
using namespace clr::metadata;

void Thread::Execute(const MethodDesc& method)
{
	Interpreter interpreter(*assemblyLoader_);

	interpreter.ExecuteMethod(method);
}
