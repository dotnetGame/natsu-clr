//
// Natsu CLR VM
//
#pragma once
#include <stack>

namespace clr
{
	namespace vm
	{
		class EvaluationStack
		{
		public:
			template<class T>
			void Push(T value)
			{
				PushImp<sizeof(value)>(reinterpret_cast<const uint8_t*>(&value));
			}

			template<class T>
			T Pop()
			{
				T value;

				auto rest = sizeof(value) % sizeof(uintptr_t);
				auto ptr = reinterpret_cast<const uint8_t*>(&value);
				if (rest)
				{
					auto last = stack_.pop();
					uintptr_t mask = 0xFF << rest * 8;
					for (size_t i = 0; i < rest; i++)
					{
						*--ptr = (last & mask) >> rest * 8;
						last << 8;
					}
				}
			}
		private:
			template<size_t N>
			void PushImp(const uint8_t* ptr)
			{
				auto icount = N / sizeof(uintptr_t);
				for (size_t i = 0; i < icount; i++)
				{
					stack_.push(*reinterpret_cast<const uintptr_t*>(ptr));
					ptr += sizeof(icount);
				}

				auto rest = sizeof(value) % sizeof(uintptr_t);
				if (rest)
				{
					uintptr_t last = 0;
					for (size_t i = 0; i < rest; i++)
						last = (last << 8) | *ptr++;
					stack_.push(rest);
				}
			}

			template<size_t N>
			void PopImp(uint8_t* ptr,)
		private:
			std::stack<uintptr_t> stack_;
		};
	}
}
