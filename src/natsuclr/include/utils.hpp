#pragma once

#define THROW_IF_NOT(expr,exception,...) {if(!(expr)) throw exception(__VA_ARGS__);}