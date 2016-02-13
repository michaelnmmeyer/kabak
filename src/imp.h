#ifndef KB_IMP_H
#define KB_IMP_H

#include <stdint.h>
#include <uchar.h>
#include <stddef.h>
#include <stdio.h>

#define local static

#define KB_MAX_CODE_POINT 0x10FFFF
#define KB_REPLACEMENT_CHAR 0xFFFD

#define KB_COMPOSE (1 << 30)
#define KB_DECOMPOSE (1 << 30)

#define KB_COMPAT KB_MERGE
#define KB_IGNORE KB_MERGE
#define KB_LUMP KB_MERGE

// Not sure if true with casefolding and custom mappings!
// FIXME should do that experimentally, using all possible option combinations.
#define KB_MAX_DECOMPOSITION 18

local size_t kb_encode_inplace(char32_t *str, size_t len);

local char32_t kb_rnext(const uint8_t *restrict, size_t, size_t *restrict);

local bool kb_code_point_valid(char32_t);

enum {
   KB_EUTF8 = 1 << 0,
   KB_EUNASSIGNED = 1 << 1,
};

#ifdef NDEBUG
   #define kb_assert(x) while (false) { (void)(x); }
#else
   #define kb_assert(x) assert(x)
#endif

#endif
