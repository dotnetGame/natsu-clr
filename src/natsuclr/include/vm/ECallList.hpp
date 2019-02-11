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

ECFuncStart(g_ArrayFuncs, "System", "Array")
	ECFuncElement("get_LongLength", &clr::corlib::Array::get_LongLength)
	ECFuncElement("get_Rank", &clr::corlib::Array::get_Rank)
	ECFuncElement("get_Length", &clr::corlib::Array::get_Length)
	ECFuncElement("GetLength", &clr::corlib::Array::GetLength)
ECFuncEnd()

ECFuncStart(g_ConsoleFuncs, "System", "Console")
	ECFuncElement("SayHello", &clr::corlib::Console::SayHello)
ECFuncEnd()

ECFuncStart(g_ObjectFuncs, "System", "Object")
	ECFuncElement("GetType", &clr::corlib::Object::GetType)
ECFuncEnd()

#undef ECFuncStart
#undef ECFuncEnd
#undef ECFuncElement
