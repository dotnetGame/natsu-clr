#include "System.Private.CoreLib.h"
#include <cmath>

using namespace natsu;
using namespace System_Private_CoreLib;
using namespace System_Private_CoreLib::System;

double Math::_s_Abs(double value)
{
    return fabs(value);
}

float Math::_s_Abs(float value)
{
    return fabsf(value);
}

double Math::_s_Acos(double value)
{
    return acos(value);
}

double Math::_s_Acosh(double value)
{
    return acosh(value);
}

double Math::_s_Asin(double value)
{
    return asin(value);
}

double Math::_s_Asinh(double value)
{
    return asinh(value);
}

double Math::_s_Atan(double value)
{
    return atan(value);
}

double Math::_s_Atan2(double y, double x)
{
    return atan2(y, x);
}

double Math::_s_Atanh(double value)
{
    return atanh(value);
}

double Math::_s_Cbrt(double value)
{
    return cbrt(value);
}

double Math::_s_Ceiling(double value)
{
    return ceil(value);
}

double Math::_s_Cos(double value)
{
    return cos(value);
}

double Math::_s_Cosh(double value)
{
    return cosh(value);
}

double Math::_s_Exp(double value)
{
    return exp(value);
}

double Math::_s_Floor(double value)
{
    return floor(value);
}

double Math::_s_Log(double value)
{
    return log(value);
}

double Math::_s_Log10(double value)
{
    return log10(value);
}

double Math::_s_Pow(double x, double y)
{
    return pow(x, y);
}

double Math::_s_Sin(double value)
{
    return sin(value);
}

double Math::_s_Sinh(double value)
{
    return sinh(value);
}

double Math::_s_Sqrt(double value)
{
    return sqrt(value);
}

double Math::_s_Tan(double value)
{
    return tan(value);
}

double Math::_s_Tanh(double value)
{
    return tanh(value);
}

double Math::_s_FMod(double x, double y)
{
    return fmod(x, y);
}

double Math::_s_ModF(double x, gc_ptr<double> y)
{
    return fmod(x, *y);
}