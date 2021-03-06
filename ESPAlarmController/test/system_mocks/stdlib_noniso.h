#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define memcpy_P(d, s, c)   memcpy(d, s,c)
#define strlen_P(s)         strlen(s)


#ifndef STDLIB_NONISO_H
#define STDLIB_NONISO_H

#ifdef __cplusplus
extern "C" {
#endif

int atoi(const char *s);

long atol(const char* s);

double atof(const char* s);

char* itoa (int val, char *s, int radix);

char* ltoa (long val, char *s, int radix);

char* utoa (unsigned int val, char *s, int radix);

char* ultoa (unsigned long val, char *s, int radix);

char* dtostrf (double val, signed char width, unsigned char prec, char *s);

#ifdef __cplusplus
} // extern "C"
#endif


#endif
