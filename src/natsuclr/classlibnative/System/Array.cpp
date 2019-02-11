//
// Natsu CLR Class Lib Native
//
#include <cassert>
#include <classlibnative/System/Array.hpp>
#include <stdio.h>

using namespace clr;
using namespace clr::corlib;

long Array::get_LongLength(ObjectRef<Array> this_)
{
    return this_->length_;
}

int Array::GetLength(ObjectRef<Array> this_, int dimension)
{
    assert(dimension == 0);
    return (int)this_->length_;
}

int Array::get_Rank(ObjectRef<Array> this_)
{
    return 1;
}

int Array::get_Length(ObjectRef<Array> this_)
{
    return (int)this_->length_;
}
