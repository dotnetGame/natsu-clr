//
// Natsu CLR VM
//

#ifndef ECFuncStart
#define ECFuncStart(funcvar, ns, cls)
#endif

#ifndef ECFuncEnd
#define ECFuncEnd()
#endif

#ifndef ECFuncElement
#define ECFuncElement(name, func)
#endif

ECFuncStart(g_ConsoleFuncs, "System", "Console")
	ECFuncElement("SayHello", &clr::corlib::Console::SayHello)
ECFuncEnd()

#undef ECFuncStart
#undef ECFuncEnd
#undef ECFuncElement
