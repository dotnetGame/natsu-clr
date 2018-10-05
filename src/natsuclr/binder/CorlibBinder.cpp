//
// Natsu CLR Binder
//
#include <binder/CorlibBinder.hpp>
#include <string_view>

using namespace std::string_view_literals;
using namespace clr;
using namespace clr::loader;
using namespace clr::vm;

static std::unique_ptr<CorlibBinder> corlibBinder_;

CorlibBinder& CorlibBinder::Current()
{
    return *corlibBinder_;
}

void CorlibBinder::Initialize(std::shared_ptr<loader::AssemblyLoader> corlibAssemblyLoader)
{
    corlibBinder_.reset(new CorlibBinder(corlibAssemblyLoader));
}

#define MAKE_STR(name) #name
#define MAKE_SV(name) MAKE_STR(name)##sv
#define INIT_TYPE(ns, typeName) Types.typeName = BindType(MAKE_SV(ns), MAKE_SV(typeName))
#define INIT_SYS_TYPE(typeName) INIT_TYPE(System, typeName)

CorlibBinder::CorlibBinder(std::shared_ptr<loader::AssemblyLoader> assemblyLoader)
    : Binder(assemblyLoader)
{
    INIT_SYS_TYPE(Boolean);
    INIT_SYS_TYPE(Double);
    INIT_SYS_TYPE(Int16);
    INIT_SYS_TYPE(Int32);
    INIT_SYS_TYPE(Object);
    INIT_SYS_TYPE(Single);
    INIT_SYS_TYPE(UInt32);
    INIT_SYS_TYPE(Void);
}
