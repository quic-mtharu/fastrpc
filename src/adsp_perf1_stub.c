// Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _ADSP_PERF1_STUB_H
#define _ADSP_PERF1_STUB_H
#include "adsp_perf1.h"
#include <string.h>
#ifndef _WIN32
#include "HAP_farf.h"
#endif //_WIN32 for HAP_farf
#include <inttypes.h>
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

#ifndef _ADSP_PERF1_SLIM_H
#define _ADSP_PERF1_SLIM_H
#include <stdint.h>

#ifndef __QAIC_SLIM
#define __QAIC_SLIM(ff) ff
#endif
#ifndef __QAIC_SLIM_EXPORT
#define __QAIC_SLIM_EXPORT
#endif

static const Type types[2];
static const Type types[2] = {
    {0x8, {{(const uintptr_t)0, (const uintptr_t)1}}, 2, 0x8},
    {0x1, {{(const uintptr_t)0, (const uintptr_t)1}}, 2, 0x1}};
static const Parameter parameters[7] = {
    {SLIM_IFPTR32(0x8, 0x10),
     {{(const uintptr_t)0x0, 0}},
     4,
     SLIM_IFPTR32(0x4, 0x8),
     0,
     0},
    {SLIM_IFPTR32(0x4, 0x8),
     {{(const uintptr_t)0xdeadc0de, (const uintptr_t)0}},
     0,
     SLIM_IFPTR32(0x4, 0x8),
     3,
     0},
    {SLIM_IFPTR32(0x4, 0x8),
     {{(const uintptr_t)0xdeadc0de, (const uintptr_t)0}},
     0,
     SLIM_IFPTR32(0x4, 0x8),
     0,
     0},
    {0x4, {{(const uintptr_t)0, (const uintptr_t)1}}, 2, 0x4, 0, 0},
    {SLIM_IFPTR32(0x8, 0x10),
     {{(const uintptr_t) & (types[0]), (const uintptr_t)0x0}},
     9,
     SLIM_IFPTR32(0x4, 0x8),
     3,
     0},
    {SLIM_IFPTR32(0x8, 0x10),
     {{(const uintptr_t) & (types[1]), (const uintptr_t)0x0}},
     9,
     SLIM_IFPTR32(0x4, 0x8),
     3,
     0},
    {0x4, {{(const uintptr_t)0, (const uintptr_t)1}}, 2, 0x4, 3, 0}};
static const Parameter *const parameterArrays[8] = {
    (&(parameters[5])), (&(parameters[6])), (&(parameters[6])),
    (&(parameters[0])), (&(parameters[1])), (&(parameters[4])),
    (&(parameters[3])), (&(parameters[2]))};
static const Method methods[5] = {
    {REMOTE_SCALARS_MAKEX(0, 0, 0x2, 0x0, 0x0, 0x1), 0x4, 0x0, 2, 2,
     (&(parameterArrays[3])), 0x4, 0x1},
    {REMOTE_SCALARS_MAKEX(0, 0, 0x0, 0x0, 0x1, 0x0), 0x0, 0x0, 1, 1,
     (&(parameterArrays[7])), 0x1, 0x0},
    {REMOTE_SCALARS_MAKEX(0, 0, 0x1, 0x0, 0x0, 0x0), 0x4, 0x0, 1, 1,
     (&(parameterArrays[6])), 0x4, 0x0},
    {REMOTE_SCALARS_MAKEX(0, 0, 0x1, 0x1, 0x0, 0x0), 0x4, 0x0, 3, 1,
     (&(parameterArrays[5])), 0x4, 0x1},
    {REMOTE_SCALARS_MAKEX(0, 0, 0x1, 0x2, 0x0, 0x0), 0x4, 0x8, 5, 3,
     (&(parameterArrays[0])), 0x4, 0x4}};
static const Method *const methodArrays[5] = {
    &(methods[0]), &(methods[1]), &(methods[2]), &(methods[3]), &(methods[4])};
static const char strings[66] =
    "get_usecs\0get_"
    "keys\0numKeys\0maxLen\0enable\0close\0open\0dst\0uri\0ix\0h\0";
static const uint16_t methodStrings[13] = {10, 14, 27, 19, 47, 56, 63,
                                           0,  52, 34, 60, 41, 63};
static const uint16_t methodStringsArrays[5] = {4, 11, 9, 7, 0};
__QAIC_SLIM_EXPORT const Interface __QAIC_SLIM(adsp_perf1_slim) = {
    5,      &(methodArrays[0]), 0, 0, &(methodStringsArrays[0]), methodStrings,
    strings};
#endif //_ADSP_PERF1_SLIM_H

#ifdef __cplusplus
extern "C" {
#endif
__QAIC_STUB_EXPORT int
__QAIC_STUB(adsp_perf1_open)(const char *uri,
                             remote_handle64 *h) __QAIC_STUB_ATTRIBUTE {
  return __QAIC_REMOTE(remote_handle64_open)(uri, h);
}
__QAIC_STUB_EXPORT int
__QAIC_STUB(adsp_perf1_close)(remote_handle64 h) __QAIC_STUB_ATTRIBUTE {
  return __QAIC_REMOTE(remote_handle64_close)(h);
}
static __inline int _stub_method(remote_handle64 _handle, uint32_t _mid,
                                 uint32_t _in0[1]) {
  remote_arg _pra[1] = {0};
  uint32_t _primIn[1] = {0};
  int _nErr = 0;
  _pra[0].buf.pv = (void *)_primIn;
  _pra[0].buf.nLen = sizeof(_primIn);
  _COPY(_primIn, 0, _in0, 0, 4);
  _TRY_FARF(_nErr,
            __QAIC_REMOTE(remote_handle64_invoke)(
                _handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 0, 0, 0), _pra));
  _CATCH_FARF(_nErr) {
    _QAIC_FARF(RUNTIME_ERROR,
               "ERROR 0x%x: handle=0x%" PRIx64
               ", scalar=0x%x, method ID=%d: %s failed\n",
               _nErr, _handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 0, 0, 0), _mid,
               __func__);
  }
  return _nErr;
}
__QAIC_STUB_EXPORT int
__QAIC_STUB(adsp_perf1_enable)(remote_handle64 _handle,
                               int ix) __QAIC_STUB_ATTRIBUTE {
  uint32_t _mid = 2;
  return _stub_method(_handle, _mid, (uint32_t *)&ix);
}
static __inline int _stub_method_1(remote_handle64 _handle, uint32_t _mid,
                                   char *_rout0[1], uint32_t _rout0Len[1]) {
  int _numIn[1] = {0};
  remote_arg _pra[2] = {0};
  uint32_t _primIn[1] = {0};
  remote_arg *_praIn = 0;
  remote_arg *_praROut = 0;
  int _nErr = 0;
  _numIn[0] = 0;
  _pra[0].buf.pv = (void *)_primIn;
  _pra[0].buf.nLen = sizeof(_primIn);
  _COPY(_primIn, 0, _rout0Len, 0, 4);
  _praIn = (_pra + 1);
  _praROut = (_praIn + _numIn[0] + 0);
  _praROut[0].buf.pv = _rout0[0];
  _praROut[0].buf.nLen = (8 * _rout0Len[0]);
  _TRY_FARF(_nErr,
            __QAIC_REMOTE(remote_handle64_invoke)(
                _handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
  _CATCH_FARF(_nErr) {
    _QAIC_FARF(RUNTIME_ERROR,
               "ERROR 0x%x: handle=0x%" PRIx64
               ", scalar=0x%x, method ID=%d: %s failed\n",
               _nErr, _handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _mid,
               __func__);
  }
  return _nErr;
}
__QAIC_STUB_EXPORT int
__QAIC_STUB(adsp_perf1_get_usecs)(remote_handle64 _handle, int64 *dst,
                                  int dstLen) __QAIC_STUB_ATTRIBUTE {
  uint32_t _mid = 3;
  return _stub_method_1(_handle, _mid, (char **)&dst, (uint32_t *)&dstLen);
}
static __inline int _stub_method_2(remote_handle64 _handle, uint32_t _mid,
                                   char *_rout0[1], uint32_t _rout0Len[1],
                                   uint32_t _rout1[1], uint32_t _rout2[1]) {
  int _numIn[1] = {0};
  remote_arg _pra[3] = {0};
  uint32_t _primIn[1] = {0};
  uint32_t _primROut[2] = {0};
  remote_arg *_praIn = 0;
  remote_arg *_praROut = 0;
  int _nErr = 0;
  _numIn[0] = 0;
  _pra[0].buf.pv = (void *)_primIn;
  _pra[0].buf.nLen = sizeof(_primIn);
  _pra[(_numIn[0] + 1)].buf.pv = (void *)_primROut;
  _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
  _COPY(_primIn, 0, _rout0Len, 0, 4);
  _praIn = (_pra + 1);
  _praROut = (_praIn + _numIn[0] + 1);
  _praROut[0].buf.pv = _rout0[0];
  _praROut[0].buf.nLen = (1 * _rout0Len[0]);
  _TRY_FARF(_nErr,
            __QAIC_REMOTE(remote_handle64_invoke)(
                _handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 2, 0, 0), _pra));
  _COPY(_rout1, 0, _primROut, 0, 4);
  _COPY(_rout2, 0, _primROut, 4, 4);
  _CATCH_FARF(_nErr) {
    _QAIC_FARF(RUNTIME_ERROR,
               "ERROR 0x%x: handle=0x%" PRIx64
               ", scalar=0x%x, method ID=%d: %s failed\n",
               _nErr, _handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 2, 0, 0), _mid,
               __func__);
  }
  return _nErr;
}
__QAIC_STUB_EXPORT int
__QAIC_STUB(adsp_perf1_get_keys)(remote_handle64 _handle, char *keys,
                                 int keysLen, int *maxLen,
                                 int *numKeys) __QAIC_STUB_ATTRIBUTE {
  uint32_t _mid = 4;
  return _stub_method_2(_handle, _mid, (char **)&keys, (uint32_t *)&keysLen,
                        (uint32_t *)maxLen, (uint32_t *)numKeys);
}
#ifdef __cplusplus
}
#endif
#endif //_ADSP_PERF1_STUB_H
