# enumPrinter

Tool to create "enumToString" and "enumFromString" functions (printf easily compatible) for enumeration from a header file

# Build & Install

make install

# Usage

enumPrinter [full path to header containing enumeration(s)]

example:
``` sh
./enumPrinter exemple.h
```
will output something like
``` c++
#ifndef EXEMPLE_ENUM_TO_STRING_H
#define EXEMPLE_ENUM_TO_STRING_H 1 

/**
    Generated using "enumPrinter - 6ab6ce0173499940"
**/

#include "exemple.h"
#include <cstring>

inline const char* enumToString(EnumA e, bool shortName = false)
{
    switch(e)
    {
    case EnumA::A:
        if (shortName) return "EnumA::A";
        return "EnumA::A";
    case EnumA::B:
        if (shortName) return "EnumA::B";
        return "EnumA::B";
    case EnumA::C:
        if (shortName) return "EnumA::C";
        return "EnumA::C";
    default:
        return "EnumA::???";
    }
}

inline bool enumFromString(const char* str, EnumA& e)
{
    if (strcmp(str, "EnumA::A") == 0)
    {
        e = EnumA::A;
        return true;
    }
    if (strcmp(str, "EnumA::B") == 0)
    {
        e = EnumA::B;
        return true;
    }
    if (strcmp(str, "EnumA::C") == 0)
    {
        e = EnumA::C;
        return true;
    }
    return false;
}

...

#endif // EXEMPLE_ENUM_TO_STRING_H
```

# Details

Supports 
- enum,
- enum class,
- typedef enum,
- enums defined into classes,
- structures and nested classes as long as enums are in public scope (if not, compile error will occur)
- namespace scope