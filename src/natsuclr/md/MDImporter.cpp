//
// Natsu CLR Metadata
//
#include <md/MDImporter.hpp>
#include <utils.hpp>
#include <bitset>
#include <cassert>
#include <limits>
#include <algorithm>

using namespace clr::loader;
using namespace clr::metadata;

#pragma pack(push, 1)

#define STORAGE_MAGIC_SIG   0x424A5342  // BSJB

struct MetadataSignature
{
	uint32_t	Signature;
	uint16_t	MarjorVersion;
	uint16_t	MinorVersion;
	uint32_t	ExtraData;
	uint32_t	VersionStringLength;
	char		VersionString[0];
};

struct MetadataHeader
{
	uint8_t		Flags;
	uint8_t		Pad0;
	uint16_t	NumberOfStreams;
};

#define MAXSTREAMNAME   32

struct StreamHeader
{
	uint32_t	Offset;
	uint32_t	Size;
	char		Name[MAXSTREAMNAME];
};

struct MetadataStreamHeader
{
	uint32_t	Reserved0;
	uint8_t		MajorVersion;
	uint8_t		MinorVersion;
	uint8_t		HeapSizes;
	uint8_t		Reserved1;
	uint64_t	Valid;
	uint64_t	Sorted;
	uint32_t	Rows[0];
};

#pragma pack(pop)

#define INIT_STREAM(field, name) (!strncmp(streamBegin->Name, name, MAXSTREAMNAME)) field.Initialize(uintptr_t(metaSig) + streamBegin->Offset)

MDImporter::MDImporter(std::shared_ptr<AssemblyFile> assemblyFile)
	:assemblyFile_(std::move(assemblyFile))
{
	auto metaSig = reinterpret_cast<const MetadataSignature*>(assemblyFile_->GetDataByRVA(assemblyFile_->GetMetadataRVA()));
	THROW_IF_NOT(metaSig->Signature == STORAGE_MAGIC_SIG, BadMetadataException, "Invalid metadata signature");
	auto metaHeader = reinterpret_cast<const MetadataHeader*>(uintptr_t(metaSig) + sizeof(MetadataSignature) + metaSig->VersionStringLength);

	auto streamBegin = reinterpret_cast<const StreamHeader*>(uintptr_t(metaHeader) + sizeof(MetadataHeader));
	for (size_t i = 0; i < metaHeader->NumberOfStreams; i++)
	{
		if INIT_STREAM(metaStream_, "#~");
		else if INIT_STREAM(stringsStream_, "#Strings");
		else if INIT_STREAM(usStream_, "#US");
		else if INIT_STREAM(guidStream_, "#GUID");
		else if INIT_STREAM(blobStream_, "#Blob");
		else THROW_IF_NOT(false, BadMetadataException, "Unrecognized stream name");
		auto headerSize = align(8 + strlen(streamBegin->Name) + 1, 4);
		streamBegin = reinterpret_cast<const StreamHeader*>(uintptr_t(streamBegin) + headerSize);
	}
}

#define INIT_TABLE_COUNT(type) if (valid[mdt_##type]) tables_[mdt_##type].reset(new type##Table(*rows++))
#define INIT_TABLE(type) if (valid[mdt_##type]) tables_[mdt_##type]->Initialize(tableContent, *this)

void MetadataStream::Initialize(uintptr_t content)
{
	auto header = reinterpret_cast<const MetadataStreamHeader*>(content);
	heapSizes_ = header->HeapSizes;
	THROW_IF_NOT(heapSizes_ == 0, BadMetadataException, "Only support tiny heaps");

	const std::bitset<64> valid(header->Valid);

	auto rows = header->Rows;
	INIT_TABLE_COUNT(Module);
	INIT_TABLE_COUNT(TypeDef);
	INIT_TABLE_COUNT(Field);
	INIT_TABLE_COUNT(MethodDef);
	INIT_TABLE_COUNT(Param);
	INIT_TABLE_COUNT(Constant);
	INIT_TABLE_COUNT(CustomAttribute);
	INIT_TABLE_COUNT(ClassLayout);
	INIT_TABLE_COUNT(StandAloneSig);
	INIT_TABLE_COUNT(PropertyMap);
	INIT_TABLE_COUNT(Property);
	INIT_TABLE_COUNT(MethodSemantics);
	INIT_TABLE_COUNT(Assembly);

	auto tableContent = uintptr_t(header->Rows) + valid.count() * sizeof(uint32_t);
	INIT_TABLE(Module);
	INIT_TABLE(TypeDef);
	INIT_TABLE(Field);
	INIT_TABLE(MethodDef);
	INIT_TABLE(Param);
	INIT_TABLE(Constant);
	INIT_TABLE(CustomAttribute);
	INIT_TABLE(ClassLayout);
	INIT_TABLE(StandAloneSig);
	INIT_TABLE(PropertyMap);
	INIT_TABLE(Property);
	INIT_TABLE(MethodSemantics);
	INIT_TABLE(Assembly);
}

size_t MetadataStream::GetSidxSize(StreamType stream) const noexcept
{
	switch (stream)
	{
	case stm_String:
	case stm_GUID:
	case stm_Blob:
		return (heapSizes_ & stream) ? sizeof(uint32_t) : sizeof(uint16_t);
	default:
		assert(!"invalid stream type");
		return 0;
	}
}

size_t MetadataStream::GetRowsCount(MetadataTables table) const noexcept
{
	auto pTable = tables_[table].get();
	return pTable ? pTable->GetCount() : 0;
}

size_t MetadataStream::GetRidxSize(MetadataTables table) const noexcept
{
	return GetRowsCount(table) > std::numeric_limits<uint16_t>::max() ? 4 : 2;
}

#define IMPL_CODED_RIDX_SIZE(ridxType) \
case ridxType: \
return GetMaxRowsCount([this](auto t) { return GetRowsCount(t); }, CodedRidx<ridxType>::PackedTypes()) >= CodedRidx<ridxType>::SizeThreshold ? 4 : 2;

template<class TCallable, MetadataTables... Types>
size_t GetMaxRowsCount(TCallable&& callable, impl::value_sequence<MetadataTables, Types...>) noexcept
{
	return std::max({ callable(Types)... });
}

size_t MetadataStream::GetCodedRidxSize(CodedRowIndex ridxType) const noexcept
{
	switch (ridxType)
	{
		IMPL_CODED_RIDX_SIZE(crid_TypeDefOrRef);
		IMPL_CODED_RIDX_SIZE(crid_HasConstant);
		IMPL_CODED_RIDX_SIZE(crid_HasCustomAttribute);
		IMPL_CODED_RIDX_SIZE(crid_CustomAttributeType);
		IMPL_CODED_RIDX_SIZE(crid_HasSemantics);
	default:
		assert(!"invalid coded row index type");
		return 0;
	}
}

#define IMPL_GET_ROW1(type) \
typename type##Table::Row MetadataStream::Get##type(Ridx<mdt_##type> ridx) const \
{																				 \
	auto table = tables_[mdt_##type].get();										 \
	THROW_IF_NOT(ridx && table, std::out_of_range, "row id out of range");		 \
	return static_cast<const type##Table*>(table)->GetRow(ridx, *this);	 \
}

IMPL_GET_ROW1(TypeDef);
IMPL_GET_ROW1(MethodDef);
IMPL_GET_ROW1(Field);
IMPL_GET_ROW1(Param);
IMPL_GET_ROW1(Constant);
IMPL_GET_ROW1(CustomAttribute);
IMPL_GET_ROW1(ClassLayout);
IMPL_GET_ROW1(StandAloneSig);
IMPL_GET_ROW1(PropertyMap);
IMPL_GET_ROW1(Property);
IMPL_GET_ROW1(MethodSemantics);

MetadataTable::MetadataTable(size_t count)
	:count_(count)
{

}

void MetadataTable::Initialize(uintptr_t& content, MetadataStream& context)
{
	base_ = content;
	rowSize_ = GetRowSize(context);
	content += rowSize_ * count_;
}

uintptr_t MetadataTable::GetRowBase(size_t index) const noexcept
{
	return base_ + (index - 1) * rowSize_;
}

// Assembly

size_t AssemblyTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 4 + 2 * 4 + context.GetSidxSize(stm_Blob) + context.GetSidxSize(stm_String);
}

// ClassLayout

size_t ClassLayoutTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 2 + 4 + context.GetRidxSize(mdt_TypeDef);
}

auto ClassLayoutTable::GetRow(Ridx<mdt_ClassLayout> ridx, const MetadataStream& context) const -> Row
{
	BinaryReader br(GetRowBase(ridx()));
	return
	{
		br.Read<uint16_t>(),
		br.Read<uint32_t>(),
		br.Read<Ridx<mdt_TypeDef>>(context.GetRidxSize(mdt_TypeDef)),
	};
}

// Constant

size_t ConstantTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 1 + 1 + context.GetCodedRidxSize(crid_HasConstant) + context.GetSidxSize(stm_Blob);
}

auto ConstantTable::GetRow(Ridx<mdt_Constant> ridx, const MetadataStream& context) const -> Row
{
	BinaryReader br(GetRowBase(ridx()));
	return
	{
		br.Read<CorElementType>(),
		0,
		br.Read<CodedRidx<crid_HasConstant>>(context.GetCodedRidxSize(crid_HasConstant)),
		br.Read<Sidx<stm_Blob>>(context.GetSidxSize(stm_Blob))
	};
}

// CustomAttribute

size_t CustomAttributeTable::GetRowSize(MetadataStream& context) const noexcept
{
	return context.GetCodedRidxSize(crid_HasCustomAttribute) + context.GetCodedRidxSize(crid_HasConstant) + context.GetSidxSize(stm_Blob);
}

auto CustomAttributeTable::GetRow(Ridx<mdt_CustomAttribute> ridx, const MetadataStream& context) const -> Row
{
	BinaryReader br(GetRowBase(ridx()));
	return
	{
		br.Read<CodedRidx<crid_HasCustomAttribute>>(context.GetCodedRidxSize(crid_HasCustomAttribute)),
		br.Read<CodedRidx<crid_HasConstant>>(context.GetCodedRidxSize(crid_HasConstant)),
		br.Read<Sidx<stm_Blob>>(context.GetSidxSize(stm_Blob))
	};
}

// Field

size_t FieldTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 2 + context.GetSidxSize(stm_String) + context.GetSidxSize(stm_Blob);
}

auto FieldTable::GetRow(Ridx<mdt_Field> ridx, const MetadataStream& context) const -> Row
{
	BinaryReader br(GetRowBase(ridx()));
	return
	{
		br.Read<FieldAttributes>(),
		br.Read<Sidx<stm_String>>(context.GetSidxSize(stm_String)),
		br.Read<Sidx<stm_Blob>>(context.GetSidxSize(stm_Blob))
	};
}

// MethodDef

size_t MethodDefTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 4 + 2 + 2 + context.GetSidxSize(stm_String) + context.GetSidxSize(stm_Blob) + context.GetRidxSize(mdt_Param);
}

auto MethodDefTable::GetRow(Ridx<mdt_MethodDef> ridx, const MetadataStream& context) const -> Row
{
	BinaryReader br(GetRowBase(ridx()));
	return
	{
		br.Read<uint32_t>(),
		br.Read<MethodImplAttributes>(),
		br.Read<MethodAttributes>(),
		br.Read<Sidx<stm_String>>(context.GetSidxSize(stm_String)),
		br.Read<Sidx<stm_Blob>>(context.GetSidxSize(stm_Blob)),
		br.Read<Ridx<mdt_Param>>(context.GetRidxSize(mdt_Param))
	};
}

// MethodSemantics

size_t MethodSemanticsTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 2 + context.GetRidxSize(mdt_MethodDef) + context.GetCodedRidxSize(crid_HasSemantics);
}

auto MethodSemanticsTable::GetRow(Ridx<mdt_MethodSemantics> ridx, const MetadataStream& context) const -> Row
{
	BinaryReader br(GetRowBase(ridx()));
	return
	{
		br.Read<MethodSemanticsAttributes>(),
		br.Read<Ridx<mdt_MethodDef>>(context.GetRidxSize(mdt_MethodDef)),
		br.Read<CodedRidx<crid_HasSemantics>>(context.GetCodedRidxSize(crid_HasSemantics))
	};
}

// Module

size_t ModuleTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 2 + context.GetSidxSize(stm_String) + context.GetSidxSize(stm_GUID) * 3;
}

// Param

size_t ParamTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 2 + 2 + context.GetSidxSize(stm_String);
}

auto ParamTable::GetRow(Ridx<mdt_Param> ridx, const MetadataStream& context) const -> Row
{
	BinaryReader br(GetRowBase(ridx()));
	return
	{
		br.Read<ParamAttributes>(),
		br.Read<uint16_t>(),
		br.Read<Sidx<stm_String>>(context.GetSidxSize(stm_String))
	};
}

// Property

size_t PropertyTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 2 + context.GetSidxSize(stm_String) + context.GetSidxSize(stm_Blob);
}

auto PropertyTable::GetRow(Ridx<mdt_Property> ridx, const MetadataStream& context) const -> Row
{
	BinaryReader br(GetRowBase(ridx()));
	return
	{
		br.Read<PropertyAttributes>(),
		br.Read<Sidx<stm_String>>(context.GetSidxSize(stm_String)),
		br.Read<Sidx<stm_Blob>>(context.GetSidxSize(stm_Blob))
	};
}

// PropertyMap

size_t PropertyMapTable::GetRowSize(MetadataStream& context) const noexcept
{
	return context.GetRidxSize(mdt_TypeDef) + context.GetRidxSize(mdt_Property);
}

auto PropertyMapTable::GetRow(Ridx<mdt_PropertyMap> ridx, const MetadataStream& context) const -> Row
{
	BinaryReader br(GetRowBase(ridx()));
	return
	{
		br.Read<Ridx<mdt_TypeDef>>(context.GetRidxSize(mdt_TypeDef)),
		br.Read<Ridx<mdt_Property>>(context.GetRidxSize(mdt_Property))
	};
}

// StandAloneSig

size_t StandAloneSigTable::GetRowSize(MetadataStream& context) const noexcept
{
	return context.GetSidxSize(stm_Blob);
}

auto StandAloneSigTable::GetRow(Ridx<mdt_StandAloneSig> ridx, const MetadataStream& context) const -> Row
{
	BinaryReader br(GetRowBase(ridx()));
	return
	{
		br.Read<Sidx<stm_Blob>>(context.GetSidxSize(stm_Blob))
	};
}

// TypeDef

size_t TypeDefTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 4 + context.GetSidxSize(stm_String) * 2 + context.GetCodedRidxSize(crid_TypeDefOrRef) + context.GetRidxSize(mdt_Field) + context.GetRidxSize(mdt_MethodDef);
}

auto TypeDefTable::GetRow(Ridx<mdt_TypeDef> ridx, const MetadataStream& context) const -> Row
{
	BinaryReader br(GetRowBase(ridx()));
	return 
	{ 
		br.Read<TypeAttributes>(),
		br.Read<Sidx<stm_String>>(context.GetSidxSize(stm_String)),
		br.Read<Sidx<stm_String>>(context.GetSidxSize(stm_String)),
		br.Read<CodedRidx<crid_TypeDefOrRef>>(context.GetCodedRidxSize(crid_TypeDefOrRef)),
		br.Read<Ridx<mdt_Field>>(context.GetRidxSize(mdt_Field)),
		br.Read<Ridx<mdt_MethodDef>>(context.GetRidxSize(mdt_MethodDef))
	};
}

void StringsStream::Initialize(uintptr_t content)
{
	content_ = reinterpret_cast<const char*>(content);
}

const char* StringsStream::GetString(Sidx<stm_String> sidx) const noexcept
{
	return content_ + sidx();
}

void USStream::Initialize(uintptr_t content)
{
	content_ = reinterpret_cast<const char*>(content);
}

void GUIDStream::Initialize(uintptr_t content)
{
	content_ = reinterpret_cast<const char*>(content);
}

void BlobStream::Initialize(uintptr_t content)
{
	content_ = reinterpret_cast<const uint8_t*>(content);
}

#define BLOB_4_BYTES_MASK uint8_t(0b11000000)
#define BLOB_2_BYTES_MASK uint8_t(0b10000000)
#define BLOB_1_BYTES_MASK uint8_t(0b00000000)
#define BLOB_SIZE_MASK uint8_t(0b11100000)

BlobData BlobStream::GetBlob(Sidx<stm_Blob> sidx) const
{
	auto base = content_ + sidx();

	size_t size;
	auto firstByte = *base++;
	switch (firstByte & BLOB_SIZE_MASK)
	{
	case BLOB_1_BYTES_MASK:
		size = firstByte & ~BLOB_1_BYTES_MASK;
		break;
	case BLOB_2_BYTES_MASK:
		size = ((firstByte & ~BLOB_2_BYTES_MASK) << 8) + *base++;
		break;
	case BLOB_4_BYTES_MASK:
		size = ((firstByte & ~BLOB_4_BYTES_MASK) << 24) + (*base++ << 16) + (*base++ << 8) + *base++;
		break;
	default:
		THROW_ALWAYS(BadMetadataException, "Invalid blob size");
		break;
	}

	return { base, size };
}
