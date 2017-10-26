/**
 * @file dynlib.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <stdio.h>

extern "C"
{

int fooo(int x, int y)
{
		return x + y;
}

};

