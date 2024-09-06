#ifndef __CORE__VECTOR_H__
#define __CORE__VECTOR_H__

#include <stdlib.h>

#define VEC_GROWTH (1 << 4)

#define Vector(T)                                                              \
    struct {                                                                   \
        T *items;                                                              \
        size_t size, capacity;                                                 \
    }

#define vec_resize(v, cap)                                                     \
    (v)->items = realloc((v)->items, ((v)->capacity = cap) *                   \
                                         sizeof(__typeof__(*(v)->items)))
#define vec_grow(v)   vec_resize(v, (v)->capacity + VEC_GROWTH)
#define vec_shrink(v) vec_resize(v, ((v)->size / VEC_GROWTH + 1) * VEC_GROWTH)

#define vec_get(v, index) index <= (v)->size ? (v)->items[index] : NULL;

#define vec_append(v, expr)                                                    \
    do {                                                                       \
        if ((v)->size == (v)->capacity)                                        \
            vec_grow(v);                                                       \
        (v)->items[(v)->size++] = (expr);                                      \
    } while (0)

#define vec_removeat(v, index)                                                 \
    do {                                                                       \
        (v)->items[index] = (v)->items[--(v)->size];                           \
        if ((v)->size > VEC_GROWTH && (v)->size <= (v)->capacity / 2)          \
            vec_shrink(v);                                                     \
    } while (0)

#define vec_remove(v, expr)                                                    \
    /* @NOTE: 'val' must be comparable (struct *wont* work, pointers will)     \
     */                                                                        \
    do {                                                                       \
        __typeof__(expr) val = (expr);                                         \
        for (size_t i = 0, size = (v)->size; size == (v)->size && i < size;    \
             ++i)                                                              \
            if ((v)->items[i] == val)                                          \
                vec_removeat(v, i);                                            \
    } while (0)

#define VEC_FOREACH(var, v)                                                    \
    /* safe to 'vec_remove{,at}' elements while iterating. */                  \
    for (int keep = 1, it = 0, size = (v)->size; keep && it < size;            \
         keep = !keep, ++it, size = (v)->size)                                 \
        for (var = (v)->items[it]; keep; keep = !keep)

#endif
