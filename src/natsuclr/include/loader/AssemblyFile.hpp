//
// Natsu CLR Loader
//
#pragma once
#include <cstdint>
#include <memory>
#include <exception>

namespace clr
{
	namespace loader
	{
		class BadImageException : std::exception
		{
			using exception::exception;
		};

		class AssemblyFile
		{
		public:
			AssemblyFile(std::shared_ptr<const uint8_t[]> data, size_t size);

			const uint8_t* GetDataByRVA(uintptr_t rva) const noexcept;
			uintptr_t GetMetadataRVA() const noexcept { return metadataRVA_; }
		private:
			std::shared_ptr<const uint8_t[]> fileData_;
			const uint8_t* textBase_;
			uintptr_t textRVA_;
			uintptr_t metadataRVA_;
		};
	}
}
