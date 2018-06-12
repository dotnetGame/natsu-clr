//
// Natsu CLR Metadata
//
#pragma once
#include "../loader/AssemblyFile.hpp"

namespace clr
{
	namespace metadata
	{
		class BadMetadataException : std::exception
		{
			using exception::exception;
		};

		typedef uint32_t Rid_t;
		typedef uint32_t Sidx_t;	// Stream Index

		enum MetadataTableIndex
		{
			mdti_Assembly = 0x20,
			mdti_AssemblyOS = 0x22,
			mdti_AssemblyProcessor = 0x21,
			mdti_MethodDef= 0x06,
			mdti_Module = 0x00,
			mdti_TypeDef = 0x02,
			mdti_Count = 0x2d
		};

		enum StreamType
		{
			stm_String = 1,
			stm_GUID = 2,
			stm_Blob = 4
		};

		class MetadataStream;

		class MetadataTable
		{
		public:
			MetadataTable(size_t count);

			void Initialize(uintptr_t& content, MetadataStream* context);
		protected:
			virtual size_t GetRowSize(MetadataStream* context) const noexcept = 0;
		private:
			uintptr_t base_;
			size_t count_;
			size_t rowSize_;
		};

		class AssemblyTable final : public MetadataTable
		{
		public:
			struct Row
			{
				uint16_t	Generation;
				Sidx_t		Name;
				Sidx_t		Mvid;
				Sidx_t		EncId;
				Sidx_t		EncBaseId;
			};

			using MetadataTable::MetadataTable;
		protected:
			virtual size_t GetRowSize(MetadataStream* context) const noexcept override;
		};

		class MethodDefTable final : public MetadataTable
		{
		public:
			struct Row
			{
				uint16_t	Generation;
				Sidx_t		Name;
				Sidx_t		Mvid;
				Sidx_t		EncId;
				Sidx_t		EncBaseId;
			};

			using MetadataTable::MetadataTable;
		protected:
			virtual size_t GetRowSize(MetadataStream* context) const noexcept override;
		};

		class ModuleTable final : public MetadataTable
		{
		public:
			struct Row
			{
				uint16_t	Generation;
				Sidx_t		Name;
				Sidx_t		Mvid;
				Sidx_t		EncId;
				Sidx_t		EncBaseId;
			};

			using MetadataTable::MetadataTable;
		protected:
			virtual size_t GetRowSize(MetadataStream* context) const noexcept override;
		};

		class TypeDefTable final : public MetadataTable
		{
		public:
			struct Row
			{
				uint16_t	Generation;
				Sidx_t		Name;
				Sidx_t		Mvid;
				Sidx_t		EncId;
				Sidx_t		EncBaseId;
			};

			using MetadataTable::MetadataTable;
		protected:
			virtual size_t GetRowSize(MetadataStream* context) const noexcept override;
		};

		class MetadataStream
		{
		public:
			void Initialize(uintptr_t content);

			size_t GetRowsCount(MetadataTableIndex table) const noexcept;
			size_t GetSidxSize(StreamType stream) const noexcept;
		private:
			std::unique_ptr<MetadataTable> tables_[mdti_Count];
			uint8_t heapSize_;
		};

		class MDImporter
		{
		public:
			MDImporter(std::shared_ptr<loader::AssemblyFile> assemblyFile);
		private:
			std::shared_ptr<loader::AssemblyFile> assemblyFile_;

			MetadataStream metaStream_;
		};
	}
}
