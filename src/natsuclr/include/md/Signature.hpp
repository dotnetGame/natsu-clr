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
		};
	}
}
