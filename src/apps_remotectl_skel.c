// Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _APPS_REMOTECTL_SKEL_H
#define _APPS_REMOTECTL_SKEL_H
#include "apps_remotectl.h"

#ifndef _QAIC_ENV_H
#define _QAIC_ENV_H

#include <stdio.h>

#ifdef __GNUC__
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#else
#pragma GCC diagnostic ignored "-Wpragmas"
#endif
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifndef _ATTRIBUTE_UNUSED

#ifdef _WIN32
#define _ATTRIBUTE_UNUSED
#else
#define _ATTRIBUTE_UNUSED __attribute__((unused))
#endif

#endif // _ATTRIBUTE_UNUSED

#ifndef _ATTRIBUTE_VISIBILITY

#ifdef _WIN32
#define _ATTRIBUTE_VISIBILITY
#else
#define _ATTRIBUTE_VISIBILITY __attribute__((visibility("default")))
#endif

#endif // _ATTRIBUTE_VISIBILITY

#ifndef __QAIC_REMOTE
#define __QAIC_REMOTE(ff) ff
#endif //__QAIC_REMOTE

#ifndef __QAIC_HEADER
#define __QAIC_HEADER(ff) ff
#endif //__QAIC_HEADER

#ifndef __QAIC_HEADER_EXPORT
#define __QAIC_HEADER_EXPORT
#endif // __QAIC_HEADER_EXPORT

#ifndef __QAIC_HEADER_ATTRIBUTE
#define __QAIC_HEADER_ATTRIBUTE
#endif // __QAIC_HEADER_ATTRIBUTE

#ifndef __QAIC_IMPL
#define __QAIC_IMPL(ff) ff
#endif //__QAIC_IMPL

#ifndef __QAIC_IMPL_EXPORT
#define __QAIC_IMPL_EXPORT
#endif // __QAIC_IMPL_EXPORT

#ifndef __QAIC_IMPL_ATTRIBUTE
#define __QAIC_IMPL_ATTRIBUTE
#endif // __QAIC_IMPL_ATTRIBUTE

#ifndef __QAIC_STUB
#define __QAIC_STUB(ff) ff
#endif //__QAIC_STUB

#ifndef __QAIC_STUB_EXPORT
#define __QAIC_STUB_EXPORT
#endif // __QAIC_STUB_EXPORT

#ifndef __QAIC_STUB_ATTRIBUTE
#define __QAIC_STUB_ATTRIBUTE
#endif // __QAIC_STUB_ATTRIBUTE

#ifndef __QAIC_SKEL
#define __QAIC_SKEL(ff) ff
#endif //__QAIC_SKEL__

#ifndef __QAIC_SKEL_EXPORT
#define __QAIC_SKEL_EXPORT
#endif // __QAIC_SKEL_EXPORT

#ifndef __QAIC_SKEL_ATTRIBUTE
#define __QAIC_SKEL_ATTRIBUTE
#endif // __QAIC_SKEL_ATTRIBUTE

#ifdef __QAIC_DEBUG__
#ifndef __QAIC_DBG_PRINTF__
#include <stdio.h>
#define __QAIC_DBG_PRINTF__(ee)                                                \
  do {                                                                         \
    printf ee;                                                                 \
  } while (0)
#endif
#else
#define __QAIC_DBG_PRINTF__(ee) (void)0
#endif

#define _OFFSET(src, sof) ((void *)(((char *)(src)) + (sof)))

#define _COPY(dst, dof, src, sof, sz)                                          \
  do {                                                                         \
    struct __copy {                                                            \
      char ar[sz];                                                             \
    };                                                                         \
    *(struct __copy *)_OFFSET(dst, dof) = *(struct __copy *)_OFFSET(src, sof); \
  } while (0)

#define _COPYIF(dst, dof, src, sof, sz)                                        \
  do {                                                                         \
    if (_OFFSET(dst, dof) != _OFFSET(src, sof)) {                              \
      _COPY(dst, dof, src, sof, sz);                                           \
    }                                                                          \
  } while (0)

_ATTRIBUTE_UNUSED
static __inline void _qaic_memmove(void *dst, void *src, int size) {
  int i = 0;
  for (i = 0; i < size; ++i) {
    ((char *)dst)[i] = ((char *)src)[i];
  }
}

#define _MEMMOVEIF(dst, src, sz)                                               \
  do {                                                                         \
    if (dst != src) {                                                          \
      _qaic_memmove(dst, src, sz);                                             \
    }                                                                          \
  } while (0)

#define _ASSIGN(dst, src, sof)                                                 \
  do {                                                                         \
    dst = OFFSET(src, sof);                                                    \
  } while (0)

#define _STD_STRLEN_IF(str) (str == 0 ? 0 : strlen(str))

#include "AEEStdErr.h"

#define _TRY(ee, func)                                                         \
  do {                                                                         \
    if (AEE_SUCCESS != ((ee) = func)) {                                        \
      __QAIC_DBG_PRINTF__(                                                     \
          (__FILE__ ":%d:error:%d:%s\n", __LINE__, (int)(ee), #func));         \
      goto ee##bail;                                                           \
    }                                                                          \
  } while (0)

#define _CATCH(exception) exception##bail : if (exception != AEE_SUCCESS)

#define _ASSERT(nErr, ff) _TRY(nErr, 0 == (ff) ? AEE_EBADPARM : AEE_SUCCESS)

#ifdef __QAIC_DEBUG__
#define _ALLOCATE(nErr, pal, size, alignment, pv)                              \
  _TRY(nErr,                                                                   \
       _allocator_alloc(pal, __FILE_LINE__, size, alignment, (void **)&pv));   \
  _ASSERT(nErr, pv || !(size))
#else
#define _ALLOCATE(nErr, pal, size, alignment, pv)                              \
  _TRY(nErr, _allocator_alloc(pal, 0, size, alignment, (void **)&pv));         \
  _ASSERT(nErr, pv || !(size))
#endif

#endif // _QAIC_ENV_H

#include <string.h>
#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include <stdlib.h>
#include <stdint.h>

typedef struct _heap _heap;
struct _heap {
  _heap *pPrev;
  const char *loc;
  uint64_t buf;
};

typedef struct _allocator {
  _heap *pheap;
  uint8_t *stack;
  uint8_t *stackEnd;
  int nSize;
} _allocator;

_ATTRIBUTE_UNUSED
static __inline int _heap_alloc(_heap **ppa, const char *loc, int size,
                                void **ppbuf) {
  _heap *pn = 0;
  pn = malloc((size_t)size + sizeof(_heap) - sizeof(uint64_t));
  if (pn != 0) {
    pn->pPrev = *ppa;
    pn->loc = loc;
    *ppa = pn;
    *ppbuf = (void *)&(pn->buf);
    return 0;
  } else {
    return -1;
  }
}
#define _ALIGN_SIZE(x, y) (((x) + (y - 1)) & ~(y - 1))

_ATTRIBUTE_UNUSED
static __inline int _allocator_alloc(_allocator *me, const char *loc, int size,
                                     unsigned int al, void **ppbuf) {
  if (size < 0) {
    return -1;
  } else if (size == 0) {
    *ppbuf = 0;
    return 0;
  }
  if ((_ALIGN_SIZE((uintptr_t)me->stackEnd, al) + (size_t)size) <
      (uintptr_t)me->stack + (size_t)me->nSize) {
    *ppbuf = (uint8_t *)_ALIGN_SIZE((uintptr_t)me->stackEnd, al);
    me->stackEnd = (uint8_t *)_ALIGN_SIZE((uintptr_t)me->stackEnd, al) + size;
    return 0;
  } else {
    return _heap_alloc(&me->pheap, loc, size, ppbuf);
  }
}

_ATTRIBUTE_UNUSED
static __inline void _allocator_deinit(_allocator *me) {
  _heap *pa = me->pheap;
  while (pa != 0) {
    _heap *pn = pa;
    const char *loc = pn->loc;
    (void)loc;
    pa = pn->pPrev;
    free(pn);
  }
}

_ATTRIBUTE_UNUSED
static __inline void _allocator_init(_allocator *me, uint8_t *stack,
                                     int stackSize) {
  me->stack = stack;
  me->stackEnd = stack + stackSize;
  me->nSize = stackSize;
  me->pheap = 0;
}

#endif // _ALLOCATOR_H

#ifndef SLIM_H
#define SLIM_H

#include <stdint.h>

// a C data structure for the idl types that can be used to implement
// static and dynamic language bindings fairly efficiently.
//
// the goal is to have a minimal ROM and RAM footprint and without
// doing too many allocations.  A good way to package these things seemed
// like the module boundary, so all the idls within  one module can share
// all the type references.

#define PARAMETER_IN 0x0
#define PARAMETER_OUT 0x1
#define PARAMETER_INOUT 0x2
#define PARAMETER_ROUT 0x3
#define PARAMETER_INROUT 0x4

// the types that we get from idl
#define TYPE_OBJECT 0x0
#define TYPE_INTERFACE 0x1
#define TYPE_PRIMITIVE 0x2
#define TYPE_ENUM 0x3
#define TYPE_STRING 0x4
#define TYPE_WSTRING 0x5
#define TYPE_STRUCTURE 0x6
#define TYPE_UNION 0x7
#define TYPE_ARRAY 0x8
#define TYPE_SEQUENCE 0x9

// these require the pack/unpack to recurse
// so it's a hint to those languages that can optimize in cases where
// recursion isn't necessary.
#define TYPE_COMPLEX_STRUCTURE (0x10 | TYPE_STRUCTURE)
#define TYPE_COMPLEX_UNION (0x10 | TYPE_UNION)
#define TYPE_COMPLEX_ARRAY (0x10 | TYPE_ARRAY)
#define TYPE_COMPLEX_SEQUENCE (0x10 | TYPE_SEQUENCE)

typedef struct Type Type;

#define INHERIT_TYPE                                                           \
  int32_t nativeSize; /*in the simple case its the same as wire size and       \
                         alignment*/                                           \
  union {                                                                      \
    struct {                                                                   \
      const uintptr_t p1;                                                      \
      const uintptr_t p2;                                                      \
    } _cast;                                                                   \
    struct {                                                                   \
      uint32_t iid;                                                            \
      uint32_t bNotNil;                                                        \
    } object;                                                                  \
    struct {                                                                   \
      const Type *arrayType;                                                   \
      int32_t nItems;                                                          \
    } array;                                                                   \
    struct {                                                                   \
      const Type *seqType;                                                     \
      int32_t nMaxLen;                                                         \
    } seqSimple;                                                               \
    struct {                                                                   \
      uint32_t bFloating;                                                      \
      uint32_t bSigned;                                                        \
    } prim;                                                                    \
    const SequenceType *seqComplex;                                            \
    const UnionType *unionType;                                                \
    const StructType *structType;                                              \
    int32_t stringMaxLen;                                                      \
    uint8_t bInterfaceNotNil;                                                  \
  } param;                                                                     \
  uint8_t type;                                                                \
  uint8_t nativeAlignment

typedef struct UnionType UnionType;
typedef struct StructType StructType;
typedef struct SequenceType SequenceType;
struct Type {
  INHERIT_TYPE;
};

struct SequenceType {
  const Type *seqType;
  uint32_t nMaxLen;
  uint32_t inSize;
  uint32_t routSizePrimIn;
  uint32_t routSizePrimROut;
};

// byte offset from the start of the case values for
// this unions case value array.  it MUST be aligned
// at the alignment requrements for the descriptor
//
// if negative it means that the unions cases are
// simple enumerators, so the value read from the descriptor
// can be used directly to find the correct case
typedef union CaseValuePtr CaseValuePtr;
union CaseValuePtr {
  const uint8_t *value8s;
  const uint16_t *value16s;
  const uint32_t *value32s;
  const uint64_t *value64s;
};

// these are only used in complex cases
// so I pulled them out of the type definition as references to make
// the type smaller
struct UnionType {
  const Type *descriptor;
  uint32_t nCases;
  const CaseValuePtr caseValues;
  const Type *const *cases;
  int32_t inSize;
  int32_t routSizePrimIn;
  int32_t routSizePrimROut;
  uint8_t inAlignment;
  uint8_t routAlignmentPrimIn;
  uint8_t routAlignmentPrimROut;
  uint8_t inCaseAlignment;
  uint8_t routCaseAlignmentPrimIn;
  uint8_t routCaseAlignmentPrimROut;
  uint8_t nativeCaseAlignment;
  uint8_t bDefaultCase;
};

struct StructType {
  uint32_t nMembers;
  const Type *const *members;
  int32_t inSize;
  int32_t routSizePrimIn;
  int32_t routSizePrimROut;
  uint8_t inAlignment;
  uint8_t routAlignmentPrimIn;
  uint8_t routAlignmentPrimROut;
};

typedef struct Parameter Parameter;
struct Parameter {
  INHERIT_TYPE;
  uint8_t mode;
  uint8_t bNotNil;
};

#define SLIM_IFPTR32(is32, is64) (sizeof(uintptr_t) == 4 ? (is32) : (is64))
#define SLIM_SCALARS_IS_DYNAMIC(u) (((u)&0x00ffffff) == 0x00ffffff)

typedef struct Method Method;
struct Method {
  uint32_t uScalars; // no method index
  int32_t primInSize;
  int32_t primROutSize;
  int maxArgs;
  int numParams;
  const Parameter *const *params;
  uint8_t primInAlignment;
  uint8_t primROutAlignment;
};

typedef struct Interface Interface;

struct Interface {
  int nMethods;
  const Method *const *methodArray;
  int nIIds;
  const uint32_t *iids;
  const uint16_t *methodStringArray;
  const uint16_t *methodStrings;
  const char *strings;
};

#endif // SLIM_H

#ifndef _APPS_REMOTECTL_SLIM_H
#define _APPS_REMOTECTL_SLIM_H
#include <stdint.h>

#ifndef __QAIC_SLIM
#define __QAIC_SLIM(ff) ff
#endif
#ifndef __QAIC_SLIM_EXPORT
#define __QAIC_SLIM_EXPORT
#endif

static const Type types[1];
static const Type types[1] = {
    {0x1, {{(const uintptr_t)0, (const uintptr_t)1}}, 2, 0x1}};
static const Parameter parameters[4] = {
    {SLIM_IFPTR32(0x8, 0x10),
     {{(const uintptr_t)0x0, 0}},
     4,
     SLIM_IFPTR32(0x4, 0x8),
     0,
     0},
    {0x4, {{(const uintptr_t)0, (const uintptr_t)1}}, 2, 0x4, 3, 0},
    {SLIM_IFPTR32(0x8, 0x10),
     {{(const uintptr_t) & (types[0]), (const uintptr_t)0x0}},
     9,
     SLIM_IFPTR32(0x4, 0x8),
     3,
     0},
    {0x4, {{(const uintptr_t)0, (const uintptr_t)1}}, 2, 0x4, 0, 0}};
static const Parameter *const parameterArrays[7] = {
    (&(parameters[0])), (&(parameters[1])), (&(parameters[2])),
    (&(parameters[1])), (&(parameters[3])), (&(parameters[2])),
    (&(parameters[1]))};
static const Method methods[2] = {
    {REMOTE_SCALARS_MAKEX(0, 0, 0x2, 0x2, 0x0, 0x0), 0x8, 0x8, 6, 4,
     (&(parameterArrays[0])), 0x4, 0x4},
    {REMOTE_SCALARS_MAKEX(0, 0, 0x1, 0x2, 0x0, 0x0), 0x8, 0x4, 5, 3,
     (&(parameterArrays[4])), 0x4, 0x4}};
static const Method *const methodArrays[2] = {&(methods[0]), &(methods[1])};
static const char strings[37] = "dlerror\0handle\0close\0nErr\0name\0open\0";
static const uint16_t methodStrings[9] = {31, 26, 8, 0, 21, 15, 8, 0, 21};
static const uint16_t methodStringsArrays[2] = {0, 5};
__QAIC_SLIM_EXPORT const Interface __QAIC_SLIM(apps_remotectl_slim) = {
    2,      &(methodArrays[0]), 0, 0, &(methodStringsArrays[0]), methodStrings,
    strings};
#endif //_APPS_REMOTECTL_SLIM_H
extern int adsp_mmap_fd_getinfo(int, uint32_t *);
#ifdef __cplusplus
extern "C" {
#endif
_ATTRIBUTE_VISIBILITY uint32_t apps_remotectl_skel_invoke_qaic_version = 10042;
static __inline int _skel_method(int (*_pfn)(int, char *, int, int *),
                                 uint32_t _sc, remote_arg *_pra) {
  remote_arg *_praEnd = 0;
  uint32_t _in0[1] = {0};
  char *_rout1[1] = {0};
  uint32_t _rout1Len[1] = {0};
  uint32_t _rout2[1] = {0};
  uint32_t *_primIn = 0;
  int _numIn[1] = {0};
  uint32_t *_primROut = 0;
  remote_arg *_praIn = 0;
  remote_arg *_praROut = 0;
  int _nErr = 0;
  _praEnd = ((_pra + REMOTE_SCALARS_INBUFS(_sc)) + REMOTE_SCALARS_OUTBUFS(_sc) +
             REMOTE_SCALARS_INHANDLES(_sc) + REMOTE_SCALARS_OUTHANDLES(_sc));
  _ASSERT(_nErr, REMOTE_SCALARS_INBUFS(_sc) == 1);
  _ASSERT(_nErr, REMOTE_SCALARS_OUTBUFS(_sc) == 2);
  _ASSERT(_nErr, REMOTE_SCALARS_INHANDLES(_sc) == 0);
  _ASSERT(_nErr, REMOTE_SCALARS_OUTHANDLES(_sc) == 0);
  _ASSERT(_nErr, (_pra + ((1 + 2) + (((0 + 0) + 0) + 0))) <= _praEnd);
  _numIn[0] = (REMOTE_SCALARS_INBUFS(_sc) - 1);
  _ASSERT(_nErr, _pra[0].buf.nLen >= 8);
  _primIn = _pra[0].buf.pv;
  _ASSERT(_nErr, _pra[(_numIn[0] + 1)].buf.nLen >= 4);
  _primROut = _pra[(_numIn[0] + 1)].buf.pv;
  _COPY(_in0, 0, _primIn, 0, 4);
  _COPY(_rout1Len, 0, _primIn, 4, 4);
  _praIn = (_pra + 1);
  _praROut = (_praIn + _numIn[0] + 1);
  _ASSERT(_nErr, ((_praROut[0].buf.nLen / 1)) >= (size_t)(_rout1Len[0]));
  _rout1[0] = _praROut[0].buf.pv;
  _TRY(_nErr,
       _pfn((int)*_in0, (char *)*_rout1, (int)*_rout1Len, (int *)_rout2));
  _COPY(_primROut, 0, _rout2, 0, 4);
  _CATCH(_nErr) {}
  return _nErr;
}
static __inline int _skel_method_1(int (*_pfn)(const char *, int *, char *, int,
                                               int *),
                                   uint32_t _sc, remote_arg *_pra) {
  remote_arg *_praEnd = 0;
  char *_in0[1] = {0};
  uint32_t _in0Len[1] = {0};
  uint32_t _rout1[1] = {0};
  char *_rout2[1] = {0};
  uint32_t _rout2Len[1] = {0};
  uint32_t _rout3[1] = {0};
  uint32_t *_primIn = 0;
  int _numIn[1] = {0};
  uint32_t *_primROut = 0;
  remote_arg *_praIn = 0;
  remote_arg *_praROut = 0;
  int _nErr = 0;
  _praEnd = ((_pra + REMOTE_SCALARS_INBUFS(_sc)) + REMOTE_SCALARS_OUTBUFS(_sc) +
             REMOTE_SCALARS_INHANDLES(_sc) + REMOTE_SCALARS_OUTHANDLES(_sc));
  _ASSERT(_nErr, REMOTE_SCALARS_INBUFS(_sc) == 2);
  _ASSERT(_nErr, REMOTE_SCALARS_OUTBUFS(_sc) == 2);
  _ASSERT(_nErr, REMOTE_SCALARS_INHANDLES(_sc) == 0);
  _ASSERT(_nErr, REMOTE_SCALARS_OUTHANDLES(_sc) == 0);
  _ASSERT(_nErr, (_pra + ((2 + 2) + (((0 + 0) + 0) + 0))) <= _praEnd);
  _numIn[0] = (REMOTE_SCALARS_INBUFS(_sc) - 1);
  _ASSERT(_nErr, _pra[0].buf.nLen >= 8);
  _primIn = _pra[0].buf.pv;
  _ASSERT(_nErr, _pra[(_numIn[0] + 1)].buf.nLen >= 8);
  _primROut = _pra[(_numIn[0] + 1)].buf.pv;
  _COPY(_in0Len, 0, _primIn, 0, 4);
  _praIn = (_pra + 1);
  _ASSERT(_nErr, ((_praIn[0].buf.nLen / 1)) >= (size_t)(_in0Len[0]));
  _in0[0] = _praIn[0].buf.pv;
  _ASSERT(_nErr, (_in0Len[0] > 0) && (_in0[0][(_in0Len[0] - 1)] == 0));
  _COPY(_rout2Len, 0, _primIn, 4, 4);
  _praROut = (_praIn + _numIn[0] + 1);
  _ASSERT(_nErr, ((_praROut[0].buf.nLen / 1)) >= (size_t)(_rout2Len[0]));
  _rout2[0] = _praROut[0].buf.pv;
  _TRY(_nErr, _pfn((const char *)*_in0, (int *)_rout1, (char *)*_rout2,
                   (int)*_rout2Len, (int *)_rout3));
  _COPY(_primROut, 0, _rout1, 0, 4);
  _COPY(_primROut, 4, _rout3, 0, 4);
  _CATCH(_nErr) {}
  return _nErr;
}
__QAIC_SKEL_EXPORT int __QAIC_SKEL(apps_remotectl_skel_invoke)(
    uint32_t _sc, remote_arg *_pra) __QAIC_SKEL_ATTRIBUTE {
  switch (REMOTE_SCALARS_METHOD(_sc)) {
  case 0:
    return _skel_method_1(__QAIC_IMPL(apps_remotectl_open), _sc, _pra);
  case 1:
    return _skel_method(__QAIC_IMPL(apps_remotectl_close), _sc, _pra);
  }
  return AEE_EUNSUPPORTED;
}
#ifdef __cplusplus
}
#endif
#endif //_APPS_REMOTECTL_SKEL_H
