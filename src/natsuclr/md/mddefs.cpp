//
// Natsu CLR Metadata
//
#include <md/mddefs.hpp>
#include <utils.hpp>

using namespace clr::metadata;

MetadataTables CodedRidx<crid_TypeDefOrRef>::GetType() const
{
	switch (GetTag())
	{
	case 0:
		return mdt_TypeDef;
	case 1:
		return mdt_TypeRef;
	case 2:
		return mdt_TypeSpec;
	default:
		THROW_ALWAYS(BadMetadataException);
	}
}
