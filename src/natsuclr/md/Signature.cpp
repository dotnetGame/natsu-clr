//
// Natsu CLR Metadata
//
#include <cassert>
#include <md/Signature.hpp>
#include <utils.hpp>

using namespace clr;
using namespace clr::metadata;

SigParser Signature::CreateParser() const noexcept
{
    return { blobData_.Data, blobData_.Length };
}

uint8_t SigParser::GetByte()
{
    auto ptr = base_;
    SkipBytes(1);
    return *ptr;
}

uint8_t SigParser::PeekByte()
{
    assert(len_);
    return *base_;
}

uint32_t SigParser::GetNumber()
{
    auto b0 = GetByte();
    assert(b0 != 0xFF);

    if ((b0 & 0x80) == 0)
        return b0;

    auto b1 = GetByte();
    if ((b0 & 0x40) == 0)
        return ((b0 & 0x3F) << 8) | b1;

    assert((b0 & 0x20) == 0);
    return ((b0 & 0x1F) << 24) | (b1 << 16) | (GetByte() << 8) | GetByte();
}

void SigParser::SkipBytes(size_t len)
{
    assert(len <= len_);
    base_ += len;
    len_ -= len;
}

bool SigParser::IsEmpty() const noexcept
{
    return len_ == 0;
}

/*

Sig ::= MethodDefSig | MethodRefSig | StandAloneMethodSig | FieldSig | PropertySig | LocalVarSig

MethodDefSig ::= [[HASTHIS] [EXPLICITTHIS]] (DEFAULT|VARARG|GENERIC GenParamCount) ParamCount RetType Param*

MethodRefSig ::= [[HASTHIS] [EXPLICITTHIS]] VARARG ParamCount RetType Param* [SENTINEL Param+]

StandAloneMethodSig ::=  [[HASTHIS] [EXPLICITTHIS]] (DEFAULT|VARARG|C|STDCALL|THISCALL|FASTCALL)
ParamCount RetType Param* [SENTINEL Param+]

FieldSig ::= FIELD CustomMod* Type

PropertySig ::= PROPERTY [HASTHIS] ParamCount CustomMod* Type Param*

LocalVarSig ::= LOCAL_SIG Count (TYPEDBYREF | ([CustomMod] [Constraint])* [BYREF] Type)+


-------------

CustomMod ::= ( CMOD_OPT | CMOD_REQD ) ( TypeDefEncoded | TypeRefEncoded )

Constraint ::= #define ELEMENT_TYPE_PINNED

Param ::= CustomMod* ( TYPEDBYREF | [BYREF] Type )

RetType ::= CustomMod* ( VOID | TYPEDBYREF | [BYREF] Type )

Type ::= ( BOOLEAN | CHAR | I1 | U1 | U2 | U2 | I4 | U4 | I8 | U8 | R4 | R8 | I | U |
| VALUETYPE TypeDefOrRefEncoded
| CLASS TypeDefOrRefEncoded
| STRING
| OBJECT
| PTR CustomMod* VOID
| PTR CustomMod* Type
| FNPTR MethodDefSig
| FNPTR MethodRefSig
| ARRAY Type ArrayShape
| SZARRAY CustomMod* Type
| GENERICINST (CLASS | VALUETYPE) TypeDefOrRefEncoded GenArgCount Type*
| VAR Number
| MVAR Number

ArrayShape ::= Rank NumSizes Size* NumLoBounds LoBound*

TypeDefOrRefEncoded ::= TypeDefEncoded | TypeRefEncoded
TypeDefEncoded ::= 32-bit-3-part-encoding-for-typedefs-and-typerefs
TypeRefEncoded ::= 32-bit-3-part-encoding-for-typedefs-and-typerefs

ParamCount ::= 29-bit-encoded-integer
GenArgCount ::= 29-bit-encoded-integer
Count ::= 29-bit-encoded-integer
Rank ::= 29-bit-encoded-integer
NumSizes ::= 29-bit-encoded-integer
Size ::= 29-bit-encoded-integer
NumLoBounds ::= 29-bit-encoded-integer
LoBounds ::= 29-bit-encoded-integer
Number ::= 29-bit-encoded-integer

*/

void SignatureVisitor::Parse(SigParser& parser)
{
    auto sigType = parser.GetByte();
    switch (sigType & 0xF)
    {
    case SIG_METHOD_DEFAULT:
    case SIG_METHOD_C:
    case SIG_METHOD_STDCALL:
    case SIG_METHOD_THISCALL:
    case SIG_METHOD_FASTCALL:
    case SIG_METHOD_VARARG:
        ParseMethod(parser, sigType);
        break;
    case SIG_FIELD:
        ParseField(parser, sigType);
        break;
    case SIG_LOCAL_SIG:
        ParseLocals(parser, sigType);
        break;
    case SIG_PROPERTY:
        ParseProperty(parser, sigType);
        break;
    default:
        THROW_ALWAYS(BadMetadataException, "Invalid signature type.");
        break;
    }
}

void SignatureVisitor::ParseMethod(SigParser& parser, uint8_t flag)
{
    VisitBeginMethod(flag);

    assert((flag & SIG_GENERIC) == 0);
    auto paramCount = parser.GetNumber();
    VisitParamCount(paramCount);

    ParseRetType(parser);
    for (size_t i = 0; i < paramCount; i++)
    {
        auto type = parser.PeekByte();
        assert(type != ELEMENT_TYPE_SENTINEL);
        ParseParam(parser);
    }

    VisitEndMethod();
}

void SignatureVisitor::ParseField(SigParser& parser, uint8_t flag)
{
    VisitBeginField(flag);

    ParseOptionalCustomMods(parser);
    ParseType(parser);

    VisitEndField();
}

void SignatureVisitor::VisitBeginField(uint8_t flag)
{
}

void SignatureVisitor::VisitEndField()
{
}

void SignatureVisitor::VisitBeginType(CorElementType elementType)
{
}

void SignatureVisitor::VisitEndType()
{
}

void SignatureVisitor::VisitTypeDefOrRefEncoded(CodedRidx<crid_TypeDefOrRef> cridx)
{
}

void SignatureVisitor::VisitBeginMethod(uint8_t flag)
{
}

void SignatureVisitor::VisitEndMethod()
{
}

void SignatureVisitor::VisitParamCount(uint32_t count)
{
}

void SignatureVisitor::VisitBeginRetType()
{
}

void SignatureVisitor::VisitEndRetType()
{
}

void SignatureVisitor::VisitBeginParam()
{
}

void SignatureVisitor::VisitEndParam()
{
}

void SignatureVisitor::VisitBeginLocalVars(uint8_t flag)
{
}

void SignatureVisitor::VisitEndLocalVars()
{
}

void SignatureVisitor::VisitLocalVarCount(uint32_t number)
{
}

void SignatureVisitor::VisitBeginLocalVar()
{
}

void SignatureVisitor::VisitEndLocalVar()
{
}

void SignatureVisitor::VisitTypeGenericParamNumber(uint32_t number)
{

}

void SignatureVisitor::ParseLocals(SigParser& parser, uint8_t flag)
{
    VisitBeginLocalVars(flag);

    auto localVarCount = parser.GetNumber();
    VisitLocalVarCount(localVarCount);

    for (size_t i = 0; i < localVarCount; i++)
    {
        ParseLocalVar(parser);
    }

    VisitEndLocalVars();
}

void SignatureVisitor::ParseProperty(SigParser& parser, uint8_t flag)
{
    assert(!"Not impl");
}

void SignatureVisitor::ParseOptionalCustomMods(SigParser& parser)
{
    while (true)
    {
        if (parser.IsEmpty())
            break;

        switch (parser.PeekByte())
        {
        case ELEMENT_TYPE_CMOD_OPT:
        case ELEMENT_TYPE_CMOD_REQD:
            ParseCustomMod(parser);
            break;
        default:
            return;
        }
    }
}

void SignatureVisitor::ParseType(SigParser& parser)
{
    auto elemType = static_cast<CorElementType>(parser.GetByte());
    VisitBeginType(elemType);

    switch (elemType)
    {
    case ELEMENT_TYPE_BOOLEAN:
    case ELEMENT_TYPE_CHAR:
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
    case ELEMENT_TYPE_R4:
    case ELEMENT_TYPE_R8:
    case ELEMENT_TYPE_I:
    case ELEMENT_TYPE_U:
    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_OBJECT:
    case ELEMENT_TYPE_TYPEDBYREF:
    case ELEMENT_TYPE_VOID:
        break;
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_VALUETYPE:
        ParseTypeDefOrRefEncoded(parser);
        break;
    case ELEMENT_TYPE_BYREF:
        ParseType(parser);
        break;
    case ELEMENT_TYPE_VAR:
        VisitTypeGenericParamNumber(parser.GetNumber());
        break;
    case ELEMENT_TYPE_PTR:
    case ELEMENT_TYPE_ARRAY:
    case ELEMENT_TYPE_GENERICINST:
    case ELEMENT_TYPE_MVAR:
        THROW_ALWAYS(NotSupportedException);
        break;
    default:
        assert(!"Not impl");
        break;
    }

    VisitEndType();
}

void SignatureVisitor::ParseCustomMod(SigParser& parser)
{
    auto cmod = parser.GetByte();

    if (cmod == ELEMENT_TYPE_CMOD_OPT || cmod == ELEMENT_TYPE_CMOD_REQD)
        ParseTypeDefOrRefEncoded(parser);

    assert(!"Invalid custom mod.");
}

void SignatureVisitor::ParseTypeDefOrRefEncoded(SigParser& parser)
{
    VisitTypeDefOrRefEncoded({ parser.GetNumber() });
}

void SignatureVisitor::ParseRetType(SigParser& parser)
{
    // RetType ::= CustomMod* ( VOID | TYPEDBYREF | [BYREF] Type )
    VisitBeginRetType();

    ParseOptionalCustomMods(parser);

    ParseType(parser);

    VisitEndRetType();
}

void SignatureVisitor::ParseParam(SigParser& parser)
{
    VisitBeginParam();

    ParseOptionalCustomMods(parser);

    ParseType(parser);

    VisitEndParam();
}

void SignatureVisitor::ParseLocalVar(SigParser& parser)
{
    VisitBeginLocalVar();

    ParseOptionalCustomMods(parser);

    ParseType(parser);

    VisitEndLocalVar();
}
