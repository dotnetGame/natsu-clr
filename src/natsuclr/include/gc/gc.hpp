//
// Natsu CLR GC
//
#pragma once
#include "../classlibnative/System/Object.hpp"
#include "../classlibnative/System/Array.hpp"
#include <vector>

namespace clr
{
namespace gc
{
    class GCHeap
    {
    public:
        GCHeap(size_t size);

        corlib::ObjectRef<> AllocateObject(vm::EEClass* cls);
        void Collect();
    private:
        std::unique_ptr<uint8_t[]> heap_;
        size_t heapSize_;
        size_t heapTop_;
    };

    class GC
    {
    public:
        static GC& Current();

        corlib::ObjectRef<> AllocateObject(vm::EEClass* cls);
        corlib::ObjectRef<corlib::Array> AllocateArray(vm::EEClass* cls, uint64_t length);

    private:
        GC();

    private:
        std::vector<GCHeap> heaps_;
    };
}
}
