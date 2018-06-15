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
	INIT_TABLE_COUNT(MethodDef);
	INIT_TABLE_COUNT(Assembly);

	auto tableContent = uintptr_t(header->Rows) + valid.count() * sizeof(uint32_t);
	INIT_TABLE(Module);
	INIT_TABLE(TypeDef);
	INIT_TABLE(MethodDef);
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
	return reinterpret_cast<const type##Table*>(table)->GetRow(ridx, *this);	 \
}

IMPL_GET_ROW1(TypeDef);

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

struct BinaryReader
{
	BinaryReader(uintptr_t base)
		:base_(base)
	{
	}

	template<class T>
	T Read() noexcept
	{
		auto offset = base_;
		base_ += sizeof(T);

		// aligned read
		if (offset % sizeof(T) == 0)
			return *reinterpret_cast<const T*>(offset);
		else
		{
			alignas(alignof(T)) uint8_t value[sizeof(T)];
			auto begin = reinterpret_cast<const uint8_t*>(offset);
			for (size_t i = 0; i < sizeof(T); i++)
				value[i] = begin[i];
			return *reinterpret_cast<const T*>(value);
		}
	}

	template<class T>
	T Read(size_t size)
	{
		if(size == 1)
			return { Read<uint8_t>() };
		else if(size == 2)
			return { Read<uint16_t>() };
		else if (size == 4)
			return { Read<uint32_t>() };
		else
			throw std::invalid_argument("invalid size");
	}
private:
	uintptr_t base_;
};

// Assembly

size_t AssemblyTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 4 + 2 * 4 + context.GetSidxSize(stm_Blob) + context.GetSidxSize(stm_String);
}

// MethodDef

size_t MethodDefTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 4 + 2 + 2 + context.GetSidxSize(stm_String) + context.GetSidxSize(stm_Blob) + context.GetRidxSize(mdt_Param);
}

// Module

size_t ModuleTable::GetRowSize(MetadataStream& context) const noexcept
{
	return 2 + context.GetSidxSize(stm_String) + context.GetSidxSize(stm_GUID) * 3;
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
	content_ = reinterpret_cast<const char*>(content);
}
