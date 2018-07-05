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
		};

		class EvaluationStack
		{
		public:
			template<class T, class = std::enable_if_t<std::is_trivially_copyable<T>::value>>
			void Push(T value)
			{
				PushImp<sizeof(value)>(reinterpret_cast<const uint8_t*>(&value));
			}

			template<class T, class = std::enable_if_t<std::is_trivially_copyable<T>::value>>
			T Pop()
			{
				T value;
				PopImp<sizeof(value)>(reinterpret_cast<uint8_t*>(&value));
				return value;
			}

			uintptr_t& GetFromTop(size_t offset)
			{
				offset = stack_.size() - offset;
				return stack_.at(offset);
			}

			void PushFrame(const MethodDesc* method, size_t argsSize, size_t retSize)
			{
				auto offset = stack_.size() - argsSize;
				stack_.resize(offset + std::max(argsSize, retSize));
				frames_.push({ method, offset, retSize });
			}

			void PopFrame()
			{
				auto& frame = frames_.top();
				stack_.resize(frame.Offset + frame.RetSize);
				frames_.pop();
			}
		private:
			template<size_t N>
			void PushImp(const uint8_t* ptr)
			{
				auto offset = stack_.size();
				auto size = align(N, sizeof(uintptr_t)) / sizeof(uintptr_t);
				stack_.resize(offset + size);
				memcpy(stack_.data() + offset, ptr, N);
			}

			template<size_t N>
			void PopImp(uint8_t* ptr)
			{
				auto size = align(N, sizeof(uintptr_t)) / sizeof(uintptr_t);
				auto offset = stack_.size() - size;
				memcpy(ptr, stack_.data() + offset, N);
				stack_.resize(offset);
			}

			constexpr size_t align(size_t value, size_t base)
			{
				auto r = value % base;
				return r ? value + (base - r) : value;
			}
		private:
			std::vector<uintptr_t> stack_;
			std::stack<FrameMarker> frames_;
		};
	}
}
