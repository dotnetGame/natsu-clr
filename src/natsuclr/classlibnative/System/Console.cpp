//
// Natsu CLR Class Lib Native
//
#include <classlibnative/System/Console.hpp>
#include <stdio.h>

using namespace clr;
using namespace clr::corlib;

void Console::SayHello(int obj)
{
	printf("%d\n", obj);
	//printf("Say Hello %s\n", obj->GetHeader().Class->TypeName);
}
