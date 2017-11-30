/**
 * @file dynlib.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <stdio.h>
#include <o3d/core/base.h>

#ifdef O3D_WINDOWS
    #define DLL_API __declspec(dllexport)
#else
    #define DLL_API
#endif

extern "C"
{

DLL_API int fooo(int x, int y)
{
    return x + y;
}

}
