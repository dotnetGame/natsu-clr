//
// Natsu CLR Metadata
//
#pragma once
#include "mddefs.hpp"

namespace clr
{
	namespace metadata
	{
		class SigParser;
		class Signature
		{
		public:
			Signature() : base_(nullptr), len_(0) {}
			Signature(const uint8_t* base, size_t len)
				:base_(base), len_(len) {}

			bool IsEmpty() const noexcept { return len_ == 0; }
			SigParser CreateParser() const noexcept;
		private:
			const uint8_t* base_;
			size_t len_;
		};

		class SigParser
		{
		public:
			SigParser(const uint8_t* base, size_t len)
				:base_(base), len_(len) {}

			uint8_t GetByte();
		private:
			void SkipBytes(size_t len);
		private:
			const uint8_t* base_;
			size_t len_;
		};
	}
}
