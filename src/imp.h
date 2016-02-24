#ifndef KB_IMP_H
#define KB_IMP_H

#include <stdint.h>
#include <uchar.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#define local static

#define KB_MAX_CODE_POINT 0x10FFFF
#define KB_BAD_CHAR (char32_t)-1
#define KB_EOF (char32_t)-2

#define KB_MAX_DECOMPOSITION 18  /* Checked by combinations.lua. */

local size_t kb_encode_inplace(char32_t *str, size_t len);

local bool kb_code_point_valid(char32_t);

local char32_t kb_sdecode(const uint8_t *str, size_t clen);

local size_t kb_decompose_char(char32_t uc,
                               char32_t dst[static KB_MAX_DECOMPOSITION],
                               unsigned options);

local void kb_canonical_reorder(char32_t *str, size_t len);

local size_t kb_compose(char32_t *buffer, size_t length, unsigned options);

local const uint8_t kb_utf8class[256];

#ifdef NDEBUG
   #define kb_assert(x) while (false) { (void)(x); }
#else
   #define kb_assert(x) assert(x)
#endif

#endif
