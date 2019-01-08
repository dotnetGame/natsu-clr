//
// Natsu CLR Metadata
//
#include <md/mddefs.hpp>
#include <utils.hpp>

using namespace clr::metadata;

MetadataTables CodedRidx<crid_TypeDefOrRef>::GetType() const
{
    switch (GetTag())
    {
    case 0:
        return mdt_TypeDef;
    case 1:
        return mdt_TypeRef;
    case 2:
        return mdt_TypeSpec;
    default:
        THROW_ALWAYS(BadMetadataException);
    }
}

MetadataTables CodedRidx<crid_HasConstant>::GetType() const
{
    switch (GetTag())
    {
    case 0:
        return mdt_Field;
    case 1:
        return mdt_Param;
    case 2:
        return mdt_Property;
    default:
        THROW_ALWAYS(BadMetadataException);
    }
}

MetadataTables CodedRidx<crid_HasCustomAttribute>::GetType() const
{
    switch (GetTag())
    {
    case 0:
        return mdt_MethodDef;
    case 1:
        return mdt_Field;
    case 2:
        return mdt_TypeRef;
    case 3:
        return mdt_TypeDef;
    case 4:
        return mdt_Param;
    case 5:
        return mdt_InterfaceImp;
    case 6:
        return mdt_MemberRef;
    case 7:
        return mdt_Module;
    case 9:
        return mdt_Property;
    case 10:
        return mdt_Event;
    case 11:
        return mdt_StandAloneSig;
    case 12:
        return mdt_ModuleRef;
    case 13:
        return mdt_TypeSpec;
    case 14:
        return mdt_Assembly;
    case 15:
        return mdt_AssemblyRef;
    case 16:
        return mdt_File;
    case 17:
        return mdt_ExportedType;
    case 18:
        return mdt_ManifestResource;
    case 19:
        return mdt_GenericParam;
    case 20:
        return mdt_GenericParamConstraint;
    case 21:
        return mdt_MethodSpec;
    default:
        THROW_ALWAYS(BadMetadataException);
    }
}

MetadataTables CodedRidx<crid_CustomAttributeType>::GetType() const
{
    switch (GetTag())
    {
    case 2:
        return mdt_MethodDef;
    case 3:
        return mdt_MemberRef;
    default:
        THROW_ALWAYS(BadMetadataException);
    }
}

MetadataTables CodedRidx<crid_HasSemantics>::GetType() const
{
    switch (GetTag())
    {
    case 0:
        return mdt_Event;
    case 1:
        return mdt_Property;
    default:
        THROW_ALWAYS(BadMetadataException);
    }
}

MetadataTables CodedRidx<crid_TypeOrMethodDef>::GetType() const
{
    switch (GetTag())
    {
    case 0:
        return mdt_TypeDef;
    case 1:
        return mdt_MethodDef;
    default:
        THROW_ALWAYS(BadMetadataException);
    }
}
