#ifndef _CORE_H
#define _CORE_H
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<stdio.h>
#include<string.h>
#include<memory.h>
#include<dirent.h>
#include<unistd.h>
#include<ctype.h>

#define DEFINE_VECTOR_OF(x) typedef struct { size_t size, cap; x* arr; }

#define VECTOR_INIT(v) do { (v).size = 0; (v).cap = 1; (v).arr = malloc(sizeof((v).arr[0])); } while(0)
#define VECTOR_FREE(v) do { free((v).arr); } while(0)
#define VECTOR_COPY(dst, src) do { (dst).size = (src).size; (dst).cap = (src).cap; (dst).arr = malloc(sizeof((dst).arr[0] * (dst).cap)); memcpy((dst).arr, (src).arr, sizeof((dst).arr[0]) * (dst).size); } while(0)
#define VECTOR_PUSH(v, o) do { if((v).size == (v).cap) (v).arr = realloc((v).arr, sizeof((v).arr[0]) * ((v).cap *= 2)); (v).arr[(v).size++] = (o); } while(0)
#define VECTOR_POP(v) ((v).arr[--(v).size]+0)
#define VECTOR_AT(v, i) (&(v).arr[i])
#define VECTOR_EL(v, i) ((v).arr[i])
#define VECTOR_ITER(v, i) size_t i = 0; i < (v).size; i++
#define VECTOR_SIZE(v) ((v).size+0)
#define VECTOR_REMOVE(v, i) do { (v).arr[i] = (v).arr[--(v).size]; } while(0)
#define VECTOR_CUT(v, i) do { if((i) != (v).size - 1) memmove(&(v).arr[i], &(v).arr[i + 1], ((v).size - (i) - 1) * sizeof((v).arr[0])); (v).size--; } while(0)

DEFINE_VECTOR_OF(uint8_t) VariableBuffer;

#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#endif /* _CORE_H */