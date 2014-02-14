#ifndef _DEFS_H
#define _DEFS_H

#include <cassert>
#include <iostream>

namespace chc
{

// #define NDEBUG

#ifdef NDEBUG
#define Debug(x)
#else
#define Debug(x) std::cout << x << std::endl
#endif

// #define NVERBOSE

#ifdef NVERBOSE
#define Print(x)
#else
#define Print(x) std::cout << x << std::endl
#endif

typedef unsigned int uint;

}

#endif
