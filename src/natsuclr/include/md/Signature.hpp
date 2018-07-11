//
// Natsu CLR Metadata
//
#pragma once
#include "mddefs.hpp"

namespace clr
{
	namespace metadata
	{
#define SIG_METHOD_DEFAULT  0x0 // default calling convention
#define SIG_METHOD_C  0x1       // C calling convention
#define SIG_METHOD_STDCALL  0x2 // Stdcall calling convention
#define SIG_METHOD_THISCALL 0x3 // thiscall  calling convention
#define SIG_METHOD_FASTCALL 0x4 // fastcall calling convention
#define SIG_METHOD_VARARG  0x5  // vararg calling convention
#define SIG_FIELD 0x6           // encodes a field
#define SIG_LOCAL_SIG 0x7       // used for the .locals directive
#define SIG_PROPERTY 0x8        // used to encode a property

#define SIG_GENERIC 0x10 // used to indicate that the method has one or more generic parameters.
#define SIG_HASTHIS 0x20  // used to encode the keyword instance in the calling convention
#define SIG_EXPLICITTHIS  0x40 // used to encode the keyword explicit in the calling convention

#define SIG_INDEX_TYPE_TYPEDEF 0    // ParseTypeDefOrRefEncoded returns this as the out index type for typedefs
#define SIG_INDEX_TYPE_TYPEREF 1    // ParseTypeDefOrRefEncoded returns this as the out index type for typerefs
#define SIG_INDEX_TYPE_TYPESPEC 2  // ParseTypeDefOrRefEncoded returns this as the out index type for typespecs

		class SigParser;
		class Signature
		{
		public:
			Signature() {}
			Signature(BlobData blobData)
				:blobData_(blobData) {}

			bool IsEmpty() const noexcept { return blobData_.Length == 0; }
			SigParser CreateParser() const noexcept;
		private:
			BlobData blobData_;
		};

		class SigParser
		{
		public:
			SigParser(const uint8_t* base, size_t len)
				:base_(base), len_(len) {}

			uint8_t GetByte();
			uint8_t PeekByte();
			uint32_t GetNumber();
			bool IsEmpty() const noexcept;
		private:
			void SkipBytes(size_t len);
		private:
			const uint8_t* base_;
			size_t len_;
		};

		class SignatureVisitor
		{
		public:
			void Parse(SigParser& parser);
		protected:
			virtual void VisitBeginField(uint8_t flag);
			virtual void VisitEndField();
			virtual void VisitTypeDefOrRefEncoded(CodedRidx<crid_TypeDefOrRef> cridx);
			virtual void VisitBeginType(CorElementType elementType);
			virtual void VisitEndType();
			virtual void VisitBeginMethod(uint8_t flag);
			virtual void VisitEndMethod();
			virtual void VisitParamCount(size_t number);
			virtual void VisitBeginRetType();
			virtual void VisitEndRetType();
			virtual void VisitBeginParam();
			virtual void VisitEndParam();
			virtual void VisitBeginLocalVars(uint8_t flag);
			virtual void VisitEndLocalVars();
			virtual void VisitLocalVarCount(size_t number);
			virtual void VisitBeginLocalVar();
			virtual void VisitEndLocalVar();
		private:
			void ParseMethod(SigParser& parser, uint8_t flag);
			void ParseField(SigParser& parser, uint8_t flag);
			void ParseLocals(SigParser& parser, uint8_t flag);
			void ParseProperty(SigParser& parser, uint8_t flag);

			void ParseOptionalCustomMods(SigParser& parser);
			void ParseType(SigParser& parser);
			void ParseCustomMod(SigParser& parser);
			void ParseTypeDefOrRefEncoded(SigParser& parser);
			void ParseRetType(SigParser& parser);
			void ParseParam(SigParser& parser);
			void ParseLocalVar(SigParser& parser);
		};
	}
}
