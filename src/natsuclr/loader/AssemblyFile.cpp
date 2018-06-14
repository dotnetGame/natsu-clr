//
// Natsu CLR Loader
//
#include <loader/AssemblyFile.hpp>
#include <utils.hpp>
#include <cstring>

using namespace clr::loader;

#pragma pack(push, 4)

#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define IMAGE_NT_SIGNATURE                  0x00004550  // PE00
#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16
#define IMAGE_FILE_MACHINE_CEE              0xC0EE

#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor

// DOS .EXE header
struct IMAGE_DOS_HEADER
{
	uint16_t   e_magic;                     // Magic number
	uint16_t   e_cblp;                      // Bytes on last page of file
	uint16_t   e_cp;                        // Pages in file
	uint16_t   e_crlc;                      // Relocations
	uint16_t   e_cparhdr;                   // Size of header in paragraphs
	uint16_t   e_minalloc;                  // Minimum extra paragraphs needed
	uint16_t   e_maxalloc;                  // Maximum extra paragraphs needed
	uint16_t   e_ss;                        // Initial (relative) SS value
	uint16_t   e_sp;                        // Initial SP value
	uint16_t   e_csum;                      // Checksum
	uint16_t   e_ip;                        // Initial IP value
	uint16_t   e_cs;                        // Initial (relative) CS value
	uint16_t   e_lfarlc;                    // File address of relocation table
	uint16_t   e_ovno;                      // Overlay number
	uint16_t   e_res[4];                    // Reserved words
	uint16_t   e_oemid;                     // OEM identifier (for e_oeminfo)
	uint16_t   e_oeminfo;                   // OEM information; e_oemid specific
	uint16_t   e_res2[10];                  // Reserved words
	uint32_t   e_lfanew;                    // File address of new exe header
};

struct IMAGE_FILE_HEADER
{
	uint16_t   Machine;
	uint16_t   NumberOfSections;
	uint32_t   TimeDateStamp;
	uint32_t   PointerToSymbolTable;
	uint32_t   NumberOfSymbols;
	uint16_t   SizeOfOptionalHeader;
	uint16_t   Characteristics;
};

struct IMAGE_DATA_DIRECTORY
{
	uint32_t   VirtualAddress;
	uint32_t   Size;
};

struct IMAGE_OPTIONAL_HEADER32
{
	//
	// Standard fields.
	//

	uint16_t   Magic;
	uint8_t    MajorLinkerVersion;
	uint8_t    MinorLinkerVersion;
	uint32_t   SizeOfCode;
	uint32_t   SizeOfInitializedData;
	uint32_t   SizeOfUninitializedData;
	uint32_t   AddressOfEntryPoint;
	uint32_t   BaseOfCode;
	uint32_t   BaseOfData;

	//
	// NT additional fields.
	//

	uint32_t   ImageBase;
	uint32_t   SectionAlignment;
	uint32_t   FileAlignment;
	uint16_t   MajorOperatingSystemVersion;
	uint16_t   MinorOperatingSystemVersion;
	uint16_t   MajorImageVersion;
	uint16_t   MinorImageVersion;
	uint16_t   MajorSubsystemVersion;
	uint16_t   MinorSubsystemVersion;
	uint32_t   Win32VersionValue;
	uint32_t   SizeOfImage;
	uint32_t   SizeOfHeaders;
	uint32_t   CheckSum;
	uint16_t   Subsystem;
	uint16_t   DllCharacteristics;
	uint32_t   SizeOfStackReserve;
	uint32_t   SizeOfStackCommit;
	uint32_t   SizeOfHeapReserve;
	uint32_t   SizeOfHeapCommit;
	uint32_t   LoaderFlags;
	uint32_t   NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

struct IMAGE_NT_HEADERS
{
	uint32_t Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER32 OptionalHeader;
};

#define IMAGE_SIZEOF_SHORT_NAME              8

struct IMAGE_SECTION_HEADER
{
	uint8_t    Name[IMAGE_SIZEOF_SHORT_NAME];
	union {
		uint32_t   PhysicalAddress;
		uint32_t   VirtualSize;
	} Misc;
	uint32_t   VirtualAddress;
	uint32_t   SizeOfRawData;
	uint32_t   PointerToRawData;
	uint32_t   PointerToRelocations;
	uint32_t   PointerToLinenumbers;
	uint16_t   NumberOfRelocations;
	uint16_t   NumberOfLinenumbers;
	uint32_t   Characteristics;
};

struct IMAGE_COR20_HEADER
{
	// Header versioning
	uint32_t                   cb;
	uint16_t                   MajorRuntimeVersion;
	uint16_t                   MinorRuntimeVersion;

	// Symbol table and startup information
	IMAGE_DATA_DIRECTORY    MetaData;
	uint32_t                   Flags;

	// The main program if it is an EXE (not used if a DLL?)
	// If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is not set, EntryPointToken represents a managed entrypoint.
	// If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is set, EntryPointRVA represents an RVA to a native entrypoint
	// (depricated for DLLs, use modules constructors intead). 
	union {
		uint32_t               EntryPointToken;
		uint32_t               EntryPointRVA;
	};

	// This is the blob of managed resources. Fetched using code:AssemblyNative.GetResource and
	// code:PEFile.GetResource and accessible from managed code from
	// System.Assembly.GetManifestResourceStream.  The meta data has a table that maps names to offsets into
	// this blob, so logically the blob is a set of resources. 
	IMAGE_DATA_DIRECTORY    Resources;
	// IL assemblies can be signed with a public-private key to validate who created it.  The signature goes
	// here if this feature is used. 
	IMAGE_DATA_DIRECTORY    StrongNameSignature;

	IMAGE_DATA_DIRECTORY    CodeManagerTable;			// Depricated, not used 
														// Used for manged codee that has unmaanaged code inside it (or exports methods as unmanaged entry points)
	IMAGE_DATA_DIRECTORY    VTableFixups;
	IMAGE_DATA_DIRECTORY    ExportAddressTableJumps;

	// null for ordinary IL images.  NGEN images it points at a code:CORCOMPILE_HEADER structure
	IMAGE_DATA_DIRECTORY    ManagedNativeHeader;
};

enum ReplacesCorHdrNumericDefines
{
	// COM+ Header entry point flags.
	COMIMAGE_FLAGS_ILONLY = 0x00000001,
	COMIMAGE_FLAGS_32BITREQUIRED = 0x00000002,    // *** Do not manipulate this bit directly (see notes above)
	COMIMAGE_FLAGS_IL_LIBRARY = 0x00000004,
	COMIMAGE_FLAGS_STRONGNAMESIGNED = 0x00000008,
	COMIMAGE_FLAGS_NATIVE_ENTRYPOINT = 0x00000010,
	COMIMAGE_FLAGS_TRACKDEBUGDATA = 0x00010000,
	COMIMAGE_FLAGS_32BITPREFERRED = 0x00020000
};

#pragma pack(pop)

static const IMAGE_SECTION_HEADER& FindSection(const char* name, const IMAGE_SECTION_HEADER* begin, const IMAGE_SECTION_HEADER* end)
{
	for (auto section = begin; section != end; section++)
	{
		if (!strcmp(name, reinterpret_cast<const char*>(section->Name)))
			return *section;
	}

	THROW_IF_NOT(false, BadImageException, "No desired section found");
}

AssemblyFile::AssemblyFile(std::shared_ptr<const uint8_t[]> data, size_t size)
	:fileData_(std::move(data))
{
	auto base = fileData_.get();
	auto dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
	THROW_IF_NOT(dosHeader->e_magic == IMAGE_DOS_SIGNATURE, BadImageException, "Invalid dos signature");
	auto ntHeader = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dosHeader->e_lfanew);
	THROW_IF_NOT(ntHeader->Signature == IMAGE_NT_SIGNATURE, BadImageException, "Invalid pe signature");
	auto& comDataDir = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];

	auto firstSection = reinterpret_cast<const IMAGE_SECTION_HEADER*>(uintptr_t(&ntHeader->OptionalHeader) + ntHeader->FileHeader.SizeOfOptionalHeader);
	auto& textSection = FindSection(".text", firstSection, firstSection + ntHeader->FileHeader.NumberOfSections);
	textBase_ = base + textSection.PointerToRawData;
	textRVA_ = textSection.VirtualAddress;

	auto corHeader = reinterpret_cast<const IMAGE_COR20_HEADER*>(GetDataByRVA(comDataDir.VirtualAddress));
	THROW_IF_NOT(corHeader->Flags == COMIMAGE_FLAGS_ILONLY, BadImageException, "Only support IL Only");
	metadataRVA_ = corHeader->MetaData.VirtualAddress;
}

const uint8_t* AssemblyFile::GetDataByRVA(uintptr_t rva) const noexcept
{
	return textBase_ + (rva - textRVA_);
}
