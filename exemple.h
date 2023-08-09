#ifndef EXEMPLE_H
#define EXEMPLE_H

class ForwardDeclaredClass;

enum EnumA
{
    A, B, C
};

struct Struture
{
    Struture() : a(0) {}

    int a;
};

typedef enum
{
    D, E, F
} EnumB;

class Class
{
public:
    enum EnumD
    {
        J, K, L
    };

    enum class EnumE
    {
        M, N, O
    };

    typedef enum
    {
        P, Q, R
    } EnumF;
};

enum class EnumC
{
    G, H, I
};

class Derived : public Class
{
public:
    enum EnumG
    {
        S, T, U
    };

    struct Struct
    {
        enum enumH
        {
            V, W, X
        };
    };
};

namespace ns
{
    namespace ns2
    {
        enum class EnumK
        {
            CC, DD
        };
    }

    enum EnumI
    {
        Y, Z
    };

    class NSClass
    {
        enum class EnumJ
        {
            AA, BB
        };
    };
} // namespace ns


#endif // EXEMPLE_H
