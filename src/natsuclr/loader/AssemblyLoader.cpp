//
// Natsu CLR Loader
//
#include <utils.hpp>
#include <loader/AssemblyLoader.hpp>
#include <loader/Layout.hpp>
#include <md/Signature.hpp>
#include <vm/ECall.hpp>
#include <cassert>

using namespace clr::loader;
using namespace clr::metadata;
using namespace clr::vm;

AssemblyLoader::AssemblyLoader(std::shared_ptr<AssemblyFile> assemblyFile)
    :mdImporter_(assemblyFile), assemblyFile_(assemblyFile)
{

}

void AssemblyLoader::Load()
{
    auto typeDefs = mdImporter_.GetTables().GetRowsCount(mdt_TypeDef);
    eeClasses_.resize(typeDefs);
    auto fields = mdImporter_.GetTables().GetRowsCount(mdt_Field);
    fieldDescs_.resize(fields);
    auto methodDefs = mdImporter_.GetTables().GetRowsCount(mdt_MethodDef);
    methodDescs_.resize(methodDefs);

    // Type
    for (uint32_t i = 0; i < typeDefs; i++)
        LoadTypeDef(i);

    // Field
    for (uint32_t i = 0; i < fields; i++)
        LoadField(i);

    for (uint32_t i = 0; i < typeDefs; i++)
        LoadTypeInstanceField(i);
    for (uint32_t i = 0; i < typeDefs; i++)
        LoadTypeStaticField(i);

    // Method
    for (uint32_t i = 0; i < methodDefs; i++)
        LoadMethodDef(i);
}

void AssemblyLoader::LoadTypeDef(uint32_t index)
{
    auto& tables = mdImporter_.GetTables();
    auto& strings = mdImporter_.GetStrings();

    auto&& eeClass = eeClasses_[index];

    eeClass.MDImporter = &mdImporter_;

    auto typeDef = tables.GetTypeDef({ index + 1 });
    eeClass.TypeName = strings.GetString(typeDef.TypeName);
    eeClass.TypeNamespace = strings.GetString(typeDef.TypeNamespace);

    auto hasNextType = index + 1 < tables.GetRowsCount(mdt_TypeDef);

    if (typeDef.MethodList)
    {
        eeClass.FirstMethod = methodDescs_.data() + typeDef.MethodList() - 1;
        if (hasNextType)
            eeClass.LastMethod = methodDescs_.data() + tables.GetTypeDef({ index + 2 }).MethodList() - 1;
        else
            eeClass.LastMethod = methodDescs_.data() + methodDescs_.size();

        for (auto method = eeClass.FirstMethod; method != eeClass.LastMethod; method++)
            method->Class = &eeClass;
    }

    if (typeDef.FieldList)
    {
        eeClass.FirstField = fieldDescs_.data() + typeDef.FieldList() - 1;
        if (hasNextType)
            eeClass.LastField = fieldDescs_.data() + tables.GetTypeDef({ index + 2 }).FieldList() - 1;
        else
            eeClass.LastField = fieldDescs_.data() + fieldDescs_.size();

        for (auto field = eeClass.FirstField; field != eeClass.LastField; field++)
            field->Class = &eeClass;
    }

    if (typeDef.Extends)
    {
        assert(typeDef.Extends.GetType() == mdt_TypeDef);
        eeClass.Parent = &GetClass(typeDef.Extends.As<mdt_TypeDef>());
    }
}

void AssemblyLoader::LoadMethodDef(uint32_t index)
{
    auto& tables = mdImporter_.GetTables();
    auto& strings = mdImporter_.GetStrings();

    auto&& method = methodDescs_[index];

    auto methodDef = tables.GetMethodDef({ index + 1 });
    method.Name = strings.GetString(methodDef.Name);

    if ((methodDef.ImplFlags & MethodImplAttributes::InternalCall) == MethodImplAttributes::InternalCall)
    {
        method.IsECall = true;

        auto& ecall = FindECall(method);
        method.ECall.EntryPoint = ecall.EntryPoint;
        method.ECall.Call = ecall.Call;
    }
    else
    {
        method.IsECall = false;
        auto headerOffset = assemblyFile_->GetDataByRVA(methodDef.RVA);
        BinaryReader br((uintptr_t)headerOffset);
        {
            enum
            {
                CorILMethod_TinyFormat = 0x2,
                CorILMethod_FatFormat = 0x3,
                CorILMethod_FormatMask = 0x3
            };

            auto flag = br.Read<uint8_t>();
            auto format = flag & CorILMethod_FormatMask;
            size_t bodyLength;
            if (format == CorILMethod_TinyFormat)
            {
                bodyLength = flag >> 2;

                method.MaxStack = 8;
                method.BodyBegin = headerOffset + 1;
            }
            else if (format == CorILMethod_FatFormat)
            {
                uint16_t fatFloags = (br.Read<uint8_t>() << 8) | flag;
                auto headerSize = (fatFloags >> 12) * 4;
                method.MaxStack = br.Read<uint16_t>();
                bodyLength = br.Read<uint32_t>();
                auto localVarSigToken = br.Read<mdToken>();

                method.BodyBegin = headerOffset + headerSize;

                if (localVarSigToken)
                {
                    assert(localVarSigToken.GetType() == mdt_StandAloneSig);
                    auto row = mdImporter_.GetTables().GetStandAloneSig(localVarSigToken.As<mdt_StandAloneSig>());
                    Signature sig(mdImporter_.GetBlobs().GetBlob(row.Signature));
                    auto localVarParser = sig.CreateParser();
                    LocalVarSigVisitor visitor;
                    visitor.AssemblyLoader = this;
                    visitor.Parse(localVarParser);

                    method.LocalVarSize = (uint32_t)LayoutVars(this, visitor.LocalVarDescs.get(), visitor.LocalVarCount);
                    method.LocalVarCount = visitor.LocalVarCount;
                    method.LocalVarDescs = std::move(visitor.LocalVarDescs);
                }
            }
            else
            {
                THROW_ALWAYS(BadImageException, "Invalid method header.");
            }

            method.BodyEnd = method.BodyBegin + bodyLength;
        }
    }

    Signature sig(mdImporter_.GetBlobs().GetBlob(methodDef.Signature));
    auto methodParser = sig.CreateParser();
    MethodSigVisitor visitor;
    visitor.AssemblyLoader = this;
    visitor.Class = method.Class;
    visitor.Parse(methodParser);

    LayoutVars(this, visitor.ParamDescs.get(), visitor.ParamCount);
    method.ParamCount = visitor.ParamCount;
    method.ParamDescs = std::move(visitor.ParamDescs);

    method.RetDesc = visitor.RetDesc.Type;
}

void AssemblyLoader::LoadField(uint32_t index)
{
    auto& tables = mdImporter_.GetTables();
    auto& strings = mdImporter_.GetStrings();

    auto& fieldDesc = fieldDescs_[index];
    auto field = tables.GetField({ index + 1 });
    fieldDesc.Ridx = { index + 1 };
    fieldDesc.Name = strings.GetString(field.Name);
    fieldDesc.Flags = field.Flags;

    Signature sig(mdImporter_.GetBlobs().GetBlob(field.Signature));
    auto sigParser = sig.CreateParser();
    FieldSigVisitor visitor;
    visitor.AssemblyLoader = this;
    visitor.Parse(sigParser);
    fieldDesc.Var = visitor.FieldVarDesc;
}

void AssemblyLoader::LoadTypeInstanceField(EEClass& eeClass)
{
    auto& tables = mdImporter_.GetTables();
    auto& strings = mdImporter_.GetStrings();

    if (eeClass.LoadLevel >= clsLoad_InstanceFields)return;

    if (eeClass.Parent && eeClass.Parent->LoadLevel < clsLoad_InstanceFields)
        LoadTypeInstanceField(*eeClass.Parent);

    auto parentSize = eeClass.Parent ? eeClass.Parent->InstanceSize : 0;
    size_t offset = parentSize;
    bool hasMyField = false;

    for (auto fieldDesc = eeClass.FirstField; fieldDesc != eeClass.LastField; ++fieldDesc)
    {
        if ((fieldDesc->Flags & FieldAttributes::Static) != FieldAttributes::Static)
        {
            hasMyField = true;

            auto& varDesc = fieldDesc->Var;
            offset = align(offset, varDesc.Type.GetAlign());
            auto size = varDesc.Type.GetStackSize();
            varDesc.Offset = uint32_t(offset);
            offset += size;
        }
    }

    if (eeClass.Parent)
        eeClass.Align = eeClass.Parent->Align;
    else
        eeClass.Align = hasMyField ? (uint32_t)eeClass.FirstField->Var.Type.GetAlign() : 1;

    eeClass.InstanceSize = (uint32_t)offset;
    eeClass.LoadLevel = clsLoad_InstanceFields;
}

void AssemblyLoader::LoadTypeInstanceField(uint32_t index)
{
    LoadTypeInstanceField(eeClasses_[index]);
}

void AssemblyLoader::LoadTypeStaticField(uint32_t index)
{
    auto& tables = mdImporter_.GetTables();
    auto& strings = mdImporter_.GetStrings();
    auto& eeClass = eeClasses_[index];

    if (eeClass.LoadLevel >= clsLoad_StaticFields)return;

    // Static & not literal field
    {
        size_t offset = 0;
        for (auto fieldDesc = eeClass.FirstField; fieldDesc != eeClass.LastField; ++fieldDesc)
        {
            auto flag = fieldDesc->Flags;
            if ((flag & FieldAttributes::Static) == FieldAttributes::Static &&
                (flag & FieldAttributes::Literal) != FieldAttributes::Literal)
            {
                auto& varDesc = fieldDesc->Var;
                offset = align(offset, varDesc.Type.GetAlign());
                auto size = varDesc.Type.GetStackSize();
                varDesc.Offset = uint32_t(offset);
                offset += size;
            }
        }

        if (offset)
            eeClass.StaticFields = std::make_unique<uint8_t[]>(offset);
    }

    eeClass.LoadLevel = clsLoad_StaticFields;
}

MethodDesc& AssemblyLoader::GetMethod(Ridx<mdt_MethodDef> method)
{
    assert(method);
    return methodDescs_[method() - 1];
}

EEClass& AssemblyLoader::GetClass(Ridx<mdt_TypeDef> type)
{
    assert(type);
    return eeClasses_[type() - 1];
}

FieldDesc& AssemblyLoader::GetField(Ridx<mdt_Field> field)
{
    assert(field);
    return fieldDescs_[field() - 1];
}
