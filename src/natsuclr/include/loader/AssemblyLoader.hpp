//
// Natsu CLR Loader
//
#pragma once
#include "AssemblyFile.hpp"
#include "../md/MDImporter.hpp"
#include "../vm/EEClass.hpp"
#include <vector>

namespace clr
{
	namespace loader
	{
		class AssemblyLoader
		{
		public:
			AssemblyLoader(std::shared_ptr<AssemblyFile> assemblyFile);

			void Load();
		private:
			void LoadTypeDef(size_t index);
			void LoadMethodDef(size_t index);
		private:
			std::shared_ptr<AssemblyFile> assemblyFile_;
			metadata::MDImporter mdImporter_;
			std::vector<vm::EEClass> eeClasses_;
			std::vector<vm::EEMethod> eeMethods_;
		};
	}
}
