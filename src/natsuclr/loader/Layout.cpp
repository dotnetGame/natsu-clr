//
// Natsu CLR Loader
//
#include <cassert>
#include <utils.hpp>
#include <loader/AssemblyLoader.hpp>
#include <loader/Layout.hpp>
#include <md/Signature.hpp>

using namespace clr::loader;
using namespace clr::metadata;
using namespace clr::vm;

void MethodSigVisitor::VisitBeginMethod(uint8_t flag)
{
    flag_ = flag;
    ParamCount = (flag_ & SIG_HASTHIS) ? 1 : 0;
}

void MethodSigVisitor::VisitBeginRetType()
{
    cntVar_ = &RetDesc;
}

void MethodSigVisitor::VisitBeginParam()
{
    if (cntParam_ == -1)
        cntVar_ = (flag_ & SIG_HASTHIS) ? &ParamDescs[1] : &ParamDescs[0];
    else
        cntVar_++;
    cntParam_++;

    cntVar_->Type.Flags |= tda_Normal;
}

void MethodSigVisitor::VisitParamCount(uint32_t count)
{
    ParamCount = (flag_ & SIG_HASTHIS) ? count + 1 : count;
    ParamDescs = std::make_unique<VarDesc[]>(ParamCount);

    if (flag_ & SIG_HASTHIS)
        ParamDescs[0].Type = { tda_Normal, ELEMENT_TYPE_CLASS, 0, Class };
}

void MethodSigVisitor::VisitBeginType(CorElementType elementType)
{
    if (elementType == ELEMENT_TYPE_BYREF)
        cntVar_->Type.Flags |= tda_ByRef;
    else
        cntVar_->Type.Type = elementType;
}

void MethodSigVisitor::VisitTypeGenericParamNumber(uint32_t number)
{
    cntVar_->Type.Flags |= tda_Generic;
    cntVar_->Type.GenericParam = &Class->GenericParams[number];
}

void MethodSigVisitor::VisitTypeDefOrRefEncoded(CodedRidx<crid_TypeDefOrRef> cridx)
{
    assert(cridx.GetType() == mdt_TypeDef);
    auto classId = cridx.As<mdt_TypeDef>();
    cntVar_->Type.Class = &AssemblyLoader->GetClasses()[classId];
}

void LocalVarSigVisitor::VisitLocalVarCount(uint32_t count)
{
    LocalVarCount = count;
    LocalVarDescs = std::make_unique<VarDesc[]>(count);
}

void LocalVarSigVisitor::VisitBeginLocalVar()
{
    if (cntVar_)
        cntVar_++;
    else
        cntVar_ = LocalVarDescs.get();
}

void LocalVarSigVisitor::VisitBeginType(CorElementType elementType)
{
    if (elementType == ELEMENT_TYPE_BYREF)
        cntVar_->Type.Flags |= tda_ByRef;
    else
        cntVar_->Type.Type = elementType;
}

void LocalVarSigVisitor::VisitTypeDefOrRefEncoded(CodedRidx<crid_TypeDefOrRef> cridx)
{
    assert(cridx.GetType() == mdt_TypeDef);
    auto classId = cridx.As<mdt_TypeDef>();
    cntVar_->Type.Class = &AssemblyLoader->GetClasses()[classId];
}

void FieldSigVisitor::VisitBeginType(CorElementType elementType)
{
    if (elementType == ELEMENT_TYPE_BYREF)
        FieldVarDesc.Type.Flags |= tda_ByRef;
    else
        FieldVarDesc.Type.Type = elementType;
}

void FieldSigVisitor::VisitTypeDefOrRefEncoded(CodedRidx<crid_TypeDefOrRef> cridx)
{
    assert(cridx.GetType() == mdt_TypeDef);
    auto classId = cridx.As<mdt_TypeDef>();
    FieldVarDesc.Type.Class = &AssemblyLoader->GetClasses()[classId];
}

size_t clr::loader::LayoutVars(AssemblyLoader* assemblyLoader, VarDesc* descs, size_t count)
{
    size_t offset = 0;
    for (size_t i = 0; i < count; i++)
    {
        auto& desc = descs[i];
        offset = align(offset, desc.Type.GetAlign());
        auto size = desc.Type.GetStackSize();
        desc.Offset = uint32_t(offset);
        offset += size;
    }

    return offset;
}
