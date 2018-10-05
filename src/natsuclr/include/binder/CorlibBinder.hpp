//
// Natsu CLR Binder
//
#pragma once
#include "Binder.hpp"

namespace clr
{
struct MethodSig
{
    vm::TypeDesc* Types;
    size_t TypesCount;
};

class CorlibBinder : public Binder
{
public:
    static CorlibBinder& Current();
    static void Initialize(std::shared_ptr<loader::AssemblyLoader> corlibAssemblyLoader);

    struct
    {
        vm::EEClass* Boolean;
        vm::EEClass* Double;
        vm::EEClass* Int16;
        vm::EEClass* Int32;
        vm::EEClass* Object;
        vm::EEClass* Single;
        vm::EEClass* UInt32;
        vm::EEClass* Void;
    } Types;

    struct
    {
    } MethodSigs;

private:
    CorlibBinder(std::shared_ptr<loader::AssemblyLoader> assemblyLoader);
};
}