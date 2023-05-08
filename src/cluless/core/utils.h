#ifndef __CORE__UTILS_H__
#define __CORE__UTILS_H__

#include <stdint.h>

typedef uint32_t State;
typedef struct {
    int x, y, w, h;
} Geometry;

#define GEOMETRY(...) ((Geometry){__VA_ARGS__})

#define LENGTH(s) (sizeof(s) / sizeof(s[0]))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define ENUM(identifier, ...) /* 'NULL' terminated enum values. */             \
    typedef enum { __VA_ARGS__, Null##identifier } identifier

#define ITER(iterable)                                                         \
    /* @NOTE: In case of nested, 'it' will repeat */                           \
    for (size_t it = 0; it < LENGTH(iterable); ++it)

#define FOREACH(var, iterable)                                                 \
    /* @NOTE: In case of nested, 'it' will repeat */                           \
    for (int keep = 1, it = 0, size = LENGTH(iterable); keep && it < size;     \
         keep = !keep, it++)                                                   \
        for (var = iterable + it; keep; keep = !keep)

#define SET(state, mask)    state |= (mask)
#define UNSET(state, mask)  state &= ~(mask)
#define TOGGLE(state, mask) state ^= (mask)
#define IS_SET(state, mask) ((state & (mask)) != 0)

#define die(...)                                                               \
    {                                                                          \
        fprintf(stderr, __VA_ARGS__);                                          \
        exit(EXIT_FAILURE);                                                    \
    }

#endif
