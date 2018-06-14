#pragma once

#define THROW_ALWAYS(exception,...) throw exception(__VA_ARGS__);
#define THROW_IF_NOT(expr,exception,...) {if(!(expr)) throw exception(__VA_ARGS__);}
