//
// Natsu CLR GC
//
#pragma once
#include "../classlibnative/System/Object.hpp"
#include <vector>

namespace clr
{
namespace gc
{
    class GCHeap
    {
    public:
        GCHeap(size_t size);

        corlib::ObjectRef<> AllocateObject(const vm::EEClass* cls);
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

        corlib::ObjectRef<> AllocateObject(const vm::EEClass* cls);

    private:
        GC();

    private:
        std::vector<GCHeap> heaps_;
    };
}
}
