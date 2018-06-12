//
// Natsu CLR Metadata
//
#pragma once
#include "../loader/AssemblyFile.hpp"
#include "mddefs.hpp"

namespace clr
{
	namespace metadata
	{
		class BadMetadataException : std::exception
		{
			using exception::exception;
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
				uint16_t			Generation;
				Sidx<stm_String>	Name;
				Sidx<stm_GUID>		Mvid;
				Sidx<stm_GUID>		EncId;
				Sidx<stm_GUID>		EncBaseId;
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
				uint16_t			Generation;
				Sidx<stm_String>	Name;
				Sidx<stm_GUID>		Mvid;
				Sidx<stm_GUID>		EncId;
				Sidx<stm_GUID>		EncBaseId;
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
				uint16_t			Generation;
				Sidx<stm_String>	Name;
				Sidx<stm_GUID>		Mvid;
				Sidx<stm_GUID>		EncId;
				Sidx<stm_GUID>		EncBaseId;
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
				TypeAttributes		Flags;
				Sidx<stm_String>	TypeName;
				Sidx<stm_String>	TypeNamespace;
				TypeDefOrRef		Extends;
				Ridx<mdt_Field>		FieldList;
				Ridx<mdt_MethodDef>	MethodList;
			};

			using MetadataTable::MetadataTable;
		protected:
			virtual size_t GetRowSize(MetadataStream* context) const noexcept override;
		};

		class MetadataStream
		{
		public:
			void Initialize(uintptr_t content);

			size_t GetRowsCount(MetadataTables table) const noexcept;
			size_t GetSidxSize(StreamType stream) const noexcept;
		private:
			std::unique_ptr<MetadataTable> tables_[mdt_Count];
			uint8_t heapSizes_;
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
