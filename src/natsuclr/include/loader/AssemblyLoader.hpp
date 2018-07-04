//
// Natsu CLR Loader
//
#pragma once
#include "AssemblyFile.hpp"
#include "../md/MDImporter.hpp"
#include "../vm/EEClass.hpp"
#include <vector>

class FieldSigVisitor;

namespace clr
{
	namespace loader
	{
		class AssemblyLoader
		{
			friend class FieldSigVisitor;
		public:
			AssemblyLoader(std::shared_ptr<AssemblyFile> assemblyFile);

			void Load();

			const std::vector<vm::EEClass>& GetClasses() const noexcept { return eeClasses_; }
			const vm::MethodDesc& GetMethod(metadata::Ridx<metadata::mdt_MethodDef> method) const;
			const vm::EEClass& GetClass (metadata::Ridx<metadata::mdt_TypeDef> type) const;
		private:
			void LoadTypeDef(size_t index);
			void LoadMethodDef(size_t index);
			void LoadField(size_t index);
			void LoadTypeInstanceField(size_t index);
		private:
			std::shared_ptr<AssemblyFile> assemblyFile_;
			metadata::MDImporter mdImporter_;
			std::vector<vm::EEClass> eeClasses_;
			std::vector<vm::MethodDesc> methodDescs_;
			std::vector<vm::FieldDesc> fieldDescs_;
		};
	}
}
