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
				PushImp(reinterpret_cast<const uint8_t*>(&value), sizeof(T), type);
			}

			template<class T, class = std::enable_if_t<std::is_trivially_copyable<T>::value>>
			EvalStackValue<T> Pop()
			{
				EvalStackValue<T> value;
				PopImp(reinterpret_cast<uint8_t*>(&value.Value), sizeof(T), value.Type);
				return value;
			}

			void PushVar(const uint8_t* ptr, size_t size, metadata::CorElementType type)
			{
				PushImp(ptr, size * sizeof(uintptr_t), type);
			}

			metadata::CorElementType PopVar(uint8_t* ptr, size_t size)
			{
				metadata::CorElementType type;
				PopImp(ptr, size * sizeof(uintptr_t), type);
				return type;
			}

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
		private:
			void PushImp(const uint8_t* ptr, size_t size, metadata::CorElementType type)
			{
				auto offset = stack_.size();
				size = align(size, sizeof(uintptr_t)) / sizeof(uintptr_t);
				stack_.resize(offset + size);
				stackType_.emplace_back(type);
				memcpy(stack_.data() + offset, ptr, size * sizeof(uintptr_t));
			}

			void PopImp(uint8_t* ptr, size_t size, metadata::CorElementType& type)
			{
				size = align(size, sizeof(uintptr_t)) / sizeof(uintptr_t);
				auto offset = stack_.size() - size;
				memcpy(ptr, stack_.data() + offset, size * sizeof(uintptr_t));
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
			StackVar GetLocalVar(size_t index);
			size_t GetLocalVarSize(size_t index);
		private:
			const MethodDesc* method_;
			EvaluationStack* stack_;
			std::vector<uintptr_t> data_;
		};
	}
}
