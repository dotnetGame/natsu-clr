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
			metadata::MDImporter& GetMDImporter() noexcept { return mdImporter_; }

			std::vector<vm::EEClass>& GetClasses() noexcept { return eeClasses_; }
			vm::MethodDesc& GetMethod(metadata::Ridx<metadata::mdt_MethodDef> method);
			vm::EEClass& GetClass (metadata::Ridx<metadata::mdt_TypeDef> type);
			vm::FieldDesc& GetField(metadata::Ridx<metadata::mdt_Field> field);
		private:
			void LoadTypeDef(size_t index);
			void LoadMethodDef(size_t index);
			void LoadField(size_t index);
			void LoadTypeInstanceField(size_t index);
			void LoadTypeStaticField(size_t index);
			void LoadTypeInstanceField(vm::EEClass& eeClass);
		private:
			std::shared_ptr<AssemblyFile> assemblyFile_;
			metadata::MDImporter mdImporter_;
			std::vector<vm::EEClass> eeClasses_;
			std::vector<vm::MethodDesc> methodDescs_;
			std::vector<vm::FieldDesc> fieldDescs_;
		};
	}
}
