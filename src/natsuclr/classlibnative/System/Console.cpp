//
// Natsu CLR Class Lib Native
//
#include <classlibnative/System/Console.hpp>
#include <stdio.h>

using namespace clr;
using namespace clr::corlib;

void Console::SayHello(float value)
{
	printf("Say Hello %f\n", value);
}
