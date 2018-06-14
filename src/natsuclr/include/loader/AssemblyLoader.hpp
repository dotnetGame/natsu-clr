//
// Natsu CLR Loader
//
#pragma once
#include "AssemblyFile.hpp"
#include "../md/MDImporter.hpp"

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
		};
	}
}
