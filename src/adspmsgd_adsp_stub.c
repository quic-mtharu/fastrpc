// Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _ADSPMSGD_ADSP_STUB_H
#define _ADSPMSGD_ADSP_STUB_H
#include "adspmsgd_adsp.h"
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

#ifndef _ADSPMSGD_ADSP_SLIM_H
#define _ADSPMSGD_ADSP_SLIM_H
#include <stdint.h>

#ifndef __QAIC_SLIM
#define __QAIC_SLIM(ff) ff
#endif
#ifndef __QAIC_SLIM_EXPORT
#define __QAIC_SLIM_EXPORT
#endif

static const Parameter parameters[3] = {
    {0x4, {{(const uintptr_t)0, (const uintptr_t)1}}, 2, 0x4, 0, 0},
    {0x4, {{(const uintptr_t)0, (const uintptr_t)1}}, 2, 0x4, 0, 0},
    {0x4, {{(const uintptr_t)0, (const uintptr_t)1}}, 2, 0x4, 3, 0}};
static const Parameter *const parameterArrays[5] = {
    (&(parameters[0])), (&(parameters[1])), (&(parameters[1])),
    (&(parameters[1])), (&(parameters[2]))};
static const Method methods[2] = {
    {REMOTE_SCALARS_MAKEX(0, 0, 0x1, 0x1, 0x0, 0x0), 0x10, 0x4, 5, 5,
     (&(parameterArrays[0])), 0x4, 0x4},
    {REMOTE_SCALARS_MAKEX(0, 0, 0x0, 0x0, 0x0, 0x0), 0x0, 0x0, 0, 0, 0, 0x0,
     0x0}};
static const Method *const methodArrays[3] = {&(methods[0]), &(methods[1]),
                                              &(methods[1])};
static const char strings[57] =
    "buff_addr\0ion_flags\0buf_size\0deinit\0filter\0heapid\0init2\0";
static const uint16_t methodStrings[8] = {31, 43, 10, 36, 20, 0, 29, 50};
static const uint16_t methodStringsArrays[3] = {0, 7, 6};
__QAIC_SLIM_EXPORT const Interface __QAIC_SLIM(adspmsgd_adsp_slim) = {
    3,      &(methodArrays[0]), 0, 0, &(methodStringsArrays[0]), methodStrings,
    strings};
#endif //_ADSPMSGD_ADSP_SLIM_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _const_adspmsgd_adsp_handle
#define _const_adspmsgd_adsp_handle ((remote_handle)-1)
#endif //_const_adspmsgd_adsp_handle

static void _adspmsgd_adsp_pls_dtor(void *data) {
  remote_handle *ph = (remote_handle *)data;
  if (_const_adspmsgd_adsp_handle != *ph) {
    (void)__QAIC_REMOTE(remote_handle_close)(*ph);
    *ph = _const_adspmsgd_adsp_handle;
  }
}

static int _adspmsgd_adsp_pls_ctor(void *ctx, void *data) {
  remote_handle *ph = (remote_handle *)data;
  *ph = _const_adspmsgd_adsp_handle;
  if (*ph == (remote_handle)-1) {
    return __QAIC_REMOTE(remote_handle_open)((const char *)ctx, ph);
  }
  return 0;
}

#if (defined __qdsp6__) || (defined __hexagon__)
#pragma weak adsp_pls_add_lookup
extern int adsp_pls_add_lookup(uint32_t type, uint32_t key, int size,
                               int (*ctor)(void *ctx, void *data), void *ctx,
                               void (*dtor)(void *ctx), void **ppo);
#pragma weak HAP_pls_add_lookup
extern int HAP_pls_add_lookup(uint32_t type, uint32_t key, int size,
                              int (*ctor)(void *ctx, void *data), void *ctx,
                              void (*dtor)(void *ctx), void **ppo);

__QAIC_STUB_EXPORT remote_handle _adspmsgd_adsp_handle(void) {
  remote_handle *ph = 0;
  if (adsp_pls_add_lookup) {
    if (0 == adsp_pls_add_lookup((uint32_t)_adspmsgd_adsp_handle, 0,
                                 sizeof(*ph), _adspmsgd_adsp_pls_ctor,
                                 "adspmsgd_adsp", _adspmsgd_adsp_pls_dtor,
                                 (void **)&ph)) {
      return *ph;
    }
    return (remote_handle)-1;
  } else if (HAP_pls_add_lookup) {
    if (0 == HAP_pls_add_lookup((uint32_t)_adspmsgd_adsp_handle, 0, sizeof(*ph),
                                _adspmsgd_adsp_pls_ctor, "adspmsgd_adsp",
                                _adspmsgd_adsp_pls_dtor, (void **)&ph)) {
      return *ph;
    }
    return (remote_handle)-1;
  }
  return (remote_handle)-1;
}

#else //__qdsp6__ || __hexagon__

uint32_t _adspmsgd_adsp_atomic_CompareAndExchange(uint32_t *volatile puDest,
                                                  uint32_t uExchange,
                                                  uint32_t uCompare);

#ifdef _WIN32
#ifdef _USRDLL
#include "Windows.h"
#else
#include "ntddk.h"
#endif //_USRDLL
uint32_t _adspmsgd_adsp_atomic_CompareAndExchange(uint32_t *volatile puDest,
                                                  uint32_t uExchange,
                                                  uint32_t uCompare) {
  return (uint32_t)InterlockedCompareExchange((volatile LONG *)puDest,
                                              (LONG)uExchange, (LONG)uCompare);
}
#elif __GNUC__
uint32_t _adspmsgd_adsp_atomic_CompareAndExchange(uint32_t *volatile puDest,
                                                  uint32_t uExchange,
                                                  uint32_t uCompare) {
  return __sync_val_compare_and_swap(puDest, uCompare, uExchange);
}
#endif //_WIN32

__QAIC_STUB_EXPORT remote_handle _adspmsgd_adsp_handle(void) {
  static remote_handle handle = _const_adspmsgd_adsp_handle;
  if ((remote_handle)-1 != handle) {
    return handle;
  } else {
    remote_handle tmp;
    int nErr = _adspmsgd_adsp_pls_ctor("adspmsgd_adsp", (void *)&tmp);
    if (nErr) {
      return (remote_handle)-1;
    }
    if (((remote_handle)-1 != handle) ||
        ((remote_handle)-1 !=
         (remote_handle)_adspmsgd_adsp_atomic_CompareAndExchange(
             (uint32_t *)&handle, (uint32_t)tmp, (uint32_t)-1))) {
      _adspmsgd_adsp_pls_dtor(&tmp);
    }
    return handle;
  }
}

#endif //__qdsp6__

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
static __inline int _stub_method(remote_handle _handle, uint32_t _mid,
                                 uint32_t _in0[1], uint32_t _in1[1],
                                 uint32_t _in2[1], uint32_t _in3[1],
                                 uint32_t _rout4[1]) {
  int _numIn[1] = {0};
  remote_arg _pra[2] = {0};
  uint32_t _primIn[4] = {0};
  uint32_t _primROut[1] = {0};
  int _nErr = 0;
  _numIn[0] = 0;
  _pra[0].buf.pv = (void *)_primIn;
  _pra[0].buf.nLen = sizeof(_primIn);
  _pra[(_numIn[0] + 1)].buf.pv = (void *)_primROut;
  _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
  _COPY(_primIn, 0, _in0, 0, 4);
  _COPY(_primIn, 4, _in1, 0, 4);
  _COPY(_primIn, 8, _in2, 0, 4);
  _COPY(_primIn, 12, _in3, 0, 4);
  _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(
                  _handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
  _COPY(_rout4, 0, _primROut, 0, 4);
  _CATCH(_nErr) {}
  return _nErr;
}
__QAIC_STUB_EXPORT int
__QAIC_STUB(adspmsgd_adsp_init)(int heapid, uint32 ion_flags, uint32 filter,
                                uint32 buf_size,
                                int *buff_addr) __QAIC_STUB_ATTRIBUTE {
  uint32_t _mid = 0;
  remote_handle _handle = _adspmsgd_adsp_handle();
  if (_handle != (remote_handle)-1) {
    return _stub_method(_handle, _mid, (uint32_t *)&heapid,
                        (uint32_t *)&ion_flags, (uint32_t *)&filter,
                        (uint32_t *)&buf_size, (uint32_t *)buff_addr);
  } else {
    return AEE_EINVHANDLE;
  }
}
static __inline int _stub_method_1(remote_handle _handle, uint32_t _mid) {
  remote_arg *_pra = 0;
  int _nErr = 0;
  _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(
                  _handle, REMOTE_SCALARS_MAKEX(0, _mid, 0, 0, 0, 0), _pra));
  _CATCH(_nErr) {}
  return _nErr;
}
__QAIC_STUB_EXPORT int
__QAIC_STUB(adspmsgd_adsp_init2)(void) __QAIC_STUB_ATTRIBUTE {
  uint32_t _mid = 1;
  remote_handle _handle = _adspmsgd_adsp_handle();
  if (_handle != (remote_handle)-1) {
    return _stub_method_1(_handle, _mid);
  } else {
    return AEE_EINVHANDLE;
  }
}
__QAIC_STUB_EXPORT int
__QAIC_STUB(adspmsgd_adsp_deinit)(void) __QAIC_STUB_ATTRIBUTE {
  uint32_t _mid = 2;
  remote_handle _handle = _adspmsgd_adsp_handle();
  if (_handle != (remote_handle)-1) {
    return _stub_method_1(_handle, _mid);
  } else {
    return AEE_EINVHANDLE;
  }
}
#ifdef __cplusplus
}
#endif
#endif //_ADSPMSGD_ADSP_STUB_H
