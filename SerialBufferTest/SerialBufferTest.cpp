#include <iostream>
#include "Profiling.h"
#include "SerialBuffer.h"

Packet p;
void Test1()
{
    float fVal = 0.1f;
    float fOut;

    PRO_BEGIN("Memcpy");
    for (int j = 0; j < 1000; j++)
    {
        p << fVal;
        p >> fOut;
    }
    PRO_END("Memcpy");
}

void Test2()
{
    int iVal = 10;
    int iOut;
    PRO_BEGIN("Shift");
    for (int j = 0; j < 1000; j++)
    {
        p << iVal;
        p >> iOut;
    }
    PRO_END("Shift");
}

struct aaa
{
    int a;
    int b;
    double c;
    char d;
};

int main()
{
    //for (int i = 0; i < 1000; i++)
    //{
    //    Test1();
    //}
    //p.Clear();
    //for (int j = 0; j < 1000; j++)
    //{
    //    Test2();
    //}
    int a, b;
    double c, d;
    char z, x;
    a = 5;
    c = 4.14;
    z = 'y';
    p << a << c << z;
    p >> b >> d >> x;
    printf("\n%d, %lf, %c\n", a, c, z);
    printf("%d, %lf, %c\n", b, d, x);

    aaa zxc = { 5,4,3.14,'A' };
    p.PutData((char*)&zxc, sizeof(aaa));
    aaa bbb;
    p.GetData((char*)&bbb, sizeof(aaa));
    printf("%d , %d, %lf, %c", bbb.a, bbb.b, bbb.c, bbb.d);
}