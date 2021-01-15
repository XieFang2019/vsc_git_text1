#include "main.h"

//函数声明
extern "C"
{
    extern void *pvPortMalloc(size_t xWantedSize);
    extern void vPortFree(void *pv);
}

//只要这个文件被编译进去，这些东西就能起效
//重载new
void *operator new(size_t size)
{
    return pvPortMalloc(size);
}

//重载delete
void operator delete(void *pointer)
{
    vPortFree(pointer);
}