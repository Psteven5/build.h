#ifndef HELLO_H_
#define HELLO_H_

#include <stdio.h>

void hello(char const *restrict subj)
{
        printf("Hello, %s!\n", subj);
}

#endif
