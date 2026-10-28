//
// Created by yan on 10/28/26.
//

#include "../Int/Util.h"

#include <stddef.h>


void DeleteString(char* str)
{
    for (size_t i = 0; i < sizeof(str); i++)
    {
        str[i] = 0;
    }
}
