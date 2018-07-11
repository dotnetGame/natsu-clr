//
// Natsu CLR VM
//
#pragma once
#include <vector>
#include <type_traits>
#include <stack>
#include <algorithm>
#include "EEClass.hpp"

namespace clr
{
	namespace vm
	{
		struct FrameMarker
		{
			const MethodDesc* Method;

			size_t Offset;
			size_t RetSize;
			size_t StackTypeOffset;
		};

		template<class T>
		struct EvalStackValue
		{
			T Value;
			metadata::CorElementType Type;
		};

		struct StackVar
		{
			uintptr_t* Data;
			metadata::CorElementType Type;
		};

		class EvaluationStack
		{
		public:
			template<class T, class = std::enable_if_t<std::is_trivially_copyable<T>::value>>
			void Push(T value, metadata::CorElementType type)
			{
				PushBytes(reinterpret_cast<const uint8_t*>(&value), sizeof(T), type);
			}

			template<class T, class = std::enable_if_t<std::is_trivially_copyable<T>::value>>
			EvalStackValue<T> Pop()
			{
				EvalStackValue<T> value;
				PopBytes(reinterpret_cast<uint8_t*>(&value.Value), sizeof(T), value.Type);
				return value;
			}

			void PushArgOrLocal(const uintptr_t* ptr, size_t size, metadata::CorElementType type)
			{
				PushBytes(reinterpret_cast<const uint8_t*>(ptr), size * sizeof(uintptr_t), type);
			}

			metadata::CorElementType PopArgOrLocal(uintptr_t* ptr, size_t size)
			{
				metadata::CorElementType type;
				PopBytes(reinterpret_cast<uint8_t*>(ptr), size * sizeof(uintptr_t), type);
				return type;
			}

			uintptr_t* GetFromTop(size_t size);

			void PushFrame(const MethodDesc* method, size_t argsSize, size_t argsCount, size_t retSize, uintptr_t* argsStore);

			void PopFrame(bool hasRet)
			{
				auto& frame = frames_.top();
				if (hasRet && frame.RetSize)
				{
					stack_.resize(frame.Offset + frame.RetSize);
					stackType_.resize(frame.StackTypeOffset + 1);
				}
				else
				{
					stack_.resize(frame.Offset);
					stackType_.resize(frame.StackTypeOffset);
				}

				frames_.pop();
			}

			metadata::CorElementType GetTopType() const
			{
				return stackType_.back();
			}

			void PushBytes(const uint8_t* ptr, size_t size, metadata::CorElementType type)
			{
				auto offset = stack_.size();
				auto alignSize = align(size, sizeof(uintptr_t));
				stack_.resize(offset + alignSize / sizeof(uintptr_t));
				stackType_.emplace_back(type);
				memcpy(stack_.data() + offset, ptr, size);
			}

			void PopBytes(uint8_t* ptr, size_t size, metadata::CorElementType& type)
			{
				auto alignSize = align(size, sizeof(uintptr_t));
				auto offset = stack_.size() - (alignSize / sizeof(uintptr_t));
				memcpy(ptr, stack_.data() + offset, size);
				stack_.resize(offset);
				type = stackType_.back();
				stackType_.pop_back();
			}

			constexpr size_t align(size_t value, size_t base)
			{
				auto r = value % base;
				return r ? value + (base - r) : value;
			}
		private:
			std::vector<uintptr_t> stack_;
			std::vector<metadata::CorElementType> stackType_;
			std::stack<FrameMarker> frames_;
		};

		class CalleeInfo
		{
		public:
			void BeginCall(const MethodDesc* method, EvaluationStack& stack);
			void EndCall(bool hasRet);

			StackVar GetArg(size_t index);
			size_t GetArgSize(size_t index);
			StackVar GetLocalVar(size_t index);
			size_t GetLocalVarSize(size_t index);
		private:
			const MethodDesc* method_;
			EvaluationStack* stack_;
			std::vector<uintptr_t> data_;
		};
	}
}
