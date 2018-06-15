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
			metadata::MDImporter mdImporter_;
			std::vector<vm::EEClass> eeClasses_;
		};
	}
}
