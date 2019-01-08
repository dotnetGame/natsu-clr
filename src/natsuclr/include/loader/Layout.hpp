//
// Natsu CLR Loader
//
#pragma once
#include "../md/MDImporter.hpp"
#include "../md/Signature.hpp"
#include "../vm/EEClass.hpp"
#include "AssemblyFile.hpp"
#include <vector>

namespace clr
{
namespace loader
{
    class AssemblyLoader;

    class MethodSigVisitor : public metadata::SignatureVisitor
    {
    public:
        AssemblyLoader* AssemblyLoader;
        vm::EEClass* Class;

        std::unique_ptr<vm::VarDesc[]> ParamDescs;
        uint32_t ParamCount = 0;

        vm::VarDesc RetDesc = {};

    protected:
        virtual void VisitBeginMethod(uint8_t flag) override;
        virtual void VisitBeginRetType() override;
        virtual void VisitBeginParam() override;
        virtual void VisitParamCount(uint32_t count) override;
        virtual void VisitBeginType(metadata::CorElementType elementType) override;
        virtual void VisitTypeDefOrRefEncoded(metadata::CodedRidx<metadata::crid_TypeDefOrRef> cridx) override;
        virtual void VisitTypeGenericParamNumber(uint32_t number) override;

    private:
        vm::VarDesc* cntVar_;
        uint8_t flag_;
        size_t cntParam_ = -1;
    };

    class LocalVarSigVisitor : public metadata::SignatureVisitor
    {
    public:
        AssemblyLoader* AssemblyLoader;

        std::unique_ptr<vm::VarDesc[]> LocalVarDescs;
        uint32_t LocalVarCount;

        virtual void VisitLocalVarCount(uint32_t count) override;
        virtual void VisitBeginLocalVar() override;
        virtual void VisitBeginType(metadata::CorElementType elementType) override;
        virtual void VisitTypeDefOrRefEncoded(metadata::CodedRidx<metadata::crid_TypeDefOrRef> cridx) override;

    private:
        vm::VarDesc* cntVar_ = nullptr;
    };

    class FieldSigVisitor : public metadata::SignatureVisitor
    {
    public:
        AssemblyLoader* AssemblyLoader;

        vm::VarDesc FieldVarDesc = {};

        virtual void VisitBeginType(metadata::CorElementType elementType) override;
        virtual void VisitTypeDefOrRefEncoded(metadata::CodedRidx<metadata::crid_TypeDefOrRef> cridx) override;
    };

    size_t LayoutVars(AssemblyLoader* assemblyLoader, vm::VarDesc* descs, size_t count);
}
}
