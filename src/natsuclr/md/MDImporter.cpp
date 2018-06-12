//
// Natsu CLR Metadata
//
#include <md/MDImporter.hpp>
#include <utils.hpp>
#include <bitset>
#include <cassert>

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
	auto metaSig = reinterpret_cast<const MetadataSignature*>(assemblyFile_->GetDataInText(assemblyFile_->GetMetadataRVA()));
	THROW_IF_NOT(metaSig->Signature == STORAGE_MAGIC_SIG, BadMetadataException, "Invalid metadata signature");
	auto metaHeader = reinterpret_cast<const MetadataHeader*>(uintptr_t(metaSig) + sizeof(MetadataSignature) + metaSig->VersionStringLength);
	
	auto streamBegin = reinterpret_cast<const StreamHeader*>(uintptr_t(metaHeader) + sizeof(MetadataHeader));
	for (size_t i = 0; i < metaHeader->NumberOfStreams; i++)
	{
		if INIT_STREAM(metaStream_, "#~");
		else THROW_IF_NOT(false, BadMetadataException, "Unrecognized stream name");
	}
}

#define INIT_TABLE_COUNT(type) if (valid[mdti_##type]) tables_[mdti_##type].reset(new type##Table(*rows++))
#define INIT_TABLE(type) if (valid[mdti_##type]) tables_[mdti_##type]->Initialize(tableContent, this)

void MetadataStream::Initialize(uintptr_t content)
{
	auto header = reinterpret_cast<const MetadataStreamHeader*>(content);
	heapSize_ = header->HeapSizes;
	THROW_IF_NOT(heapSize_ == 0, BadMetadataException, "Only support tiny heaps");

	const std::bitset<64> valid(header->Valid);

	auto rows = header->Rows;
	INIT_TABLE_COUNT(Module);

	auto tableContent = uintptr_t(header->Rows) + valid.count() * sizeof(uint32_t);
	INIT_TABLE(Module);
}

size_t MetadataStream::GetSidxSize(StreamType stream) const noexcept
{
	switch (stream)
	{
	case stm_String:
	case stm_GUID:
	case stm_Blob:
		return (heapSize_ & stream) ? sizeof(uint32_t) : sizeof(uint16_t);
	default:
		assert(!"invalid stream type");
		return 0;
	}
}

MetadataTable::MetadataTable(size_t count)
	:count_(count)
{

}

void MetadataTable::Initialize(uintptr_t& content, MetadataStream* context)
{
	base_ = content;
	rowSize_ = GetRowSize(context);
	content += rowSize_ * count_;
}

size_t ModuleTable::GetRowSize(MetadataStream* context) const noexcept
{
	return sizeof(Row::Generation) + context->GetSidxSize(stm_String) + context->GetSidxSize(stm_GUID) * 3;
}
