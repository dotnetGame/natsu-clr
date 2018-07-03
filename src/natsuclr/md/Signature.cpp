//
// Natsu CLR Metadata
//
#include <md/Signature.hpp>
#include <cassert>

using namespace clr;
using namespace clr::metadata;

SigParser Signature::CreateParser() const noexcept
{
	return { base_, len_ };
}

uint8_t SigParser::GetByte()
{
	auto ptr = base_;
	SkipBytes(1);
	return *ptr;
}

void SigParser::SkipBytes(size_t len)
{
	assert(len <= len_);
	base_ += len;
	len_ -= len;
}
