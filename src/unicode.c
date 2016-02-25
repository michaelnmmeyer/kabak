#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include "api.h"
#include "imp.h"

/* Once tables are created, we only need to know whether the decomposition is
 * canonical or not. Hence the definitions below.
 */
#define KB_DECOMP_TYPE_FONT      1
#define KB_DECOMP_TYPE_NOBREAK   1
#define KB_DECOMP_TYPE_INITIAL   1
#define KB_DECOMP_TYPE_MEDIAL    1
#define KB_DECOMP_TYPE_FINAL     1
#define KB_DECOMP_TYPE_ISOLATED  1
#define KB_DECOMP_TYPE_CIRCLE    1
#define KB_DECOMP_TYPE_SUPER     1
#define KB_DECOMP_TYPE_SUB       1
#define KB_DECOMP_TYPE_VERTICAL 1
#define KB_DECOMP_TYPE_WIDE     1
#define KB_DECOMP_TYPE_NARROW   1
#define KB_DECOMP_TYPE_SMALL    1
#define KB_DECOMP_TYPE_SQUARE   1
#define KB_DECOMP_TYPE_FRACTION 1
#define KB_DECOMP_TYPE_COMPAT   1

struct kb_property {
   uint16_t category;            /* Could group categories to use less space. */
   uint16_t decomp_mapping;
   uint16_t casefold_mapping;
   int16_t comb1st_index;
   int16_t comb2nd_index;        /* Can be made smaller. */
   uint16_t combining_class;     /* Can be stuffed in one byte. */
   unsigned decomp_type:1;       /* True iff a compatibility mapping. */
   unsigned comp_exclusion:1;
   unsigned ignorable:1;
};

#include "data.ic"

#define KB_HANGUL_SBASE 0xAC00
#define KB_HANGUL_LBASE 0x1100
#define KB_HANGUL_VBASE 0x1161
#define KB_HANGUL_TBASE 0x11A7
#define KB_HANGUL_SCOUNT 11172
#define KB_HANGUL_LCOUNT 19
#define KB_HANGUL_VCOUNT 21
#define KB_HANGUL_TCOUNT 28
#define KB_HANGUL_NCOUNT 588

local bool kb_code_point_valid(char32_t c)
{
   return c < 0xD800 || (c > 0xDFFF && c <= KB_MAX_CODE_POINT);
}

static const struct kb_property *kb_get_property(char32_t c)
{
   size_t idx = kb_stage2table[kb_stage1table[c >> 8] + (c & 0xFF)];
   return &kb_properties[idx];
}

#define kb_decompose_lump(replacement_uc) \
   kb_decompose_char((replacement_uc), dst, options & ~KB_LUMP)

static size_t kb_decompose_hangul(char32_t uc, char32_t dst[static 3])
{
   char32_t hangul_sindex = uc - KB_HANGUL_SBASE;
   if (hangul_sindex < KB_HANGUL_SCOUNT) {
      dst[0] = KB_HANGUL_LBASE + hangul_sindex / KB_HANGUL_NCOUNT;
      dst[1] = KB_HANGUL_VBASE +
               (hangul_sindex % KB_HANGUL_NCOUNT) / KB_HANGUL_TCOUNT;
      char32_t hangul_tindex = hangul_sindex % KB_HANGUL_TCOUNT;
      if (!hangul_tindex)
         return 2;
      dst[2] = KB_HANGUL_TBASE + hangul_tindex;
      return 3;
   }
   return 0;
}

static size_t kb_decompose_char(char32_t uc,
                                char32_t dst[static KB_MAX_DECOMPOSITION],
                                unsigned options);

static size_t kb_decompose_seq(char32_t *restrict dst, size_t idx, unsigned opts)
{
   struct kb_sequence seq = kb_sequences[idx];

   size_t nr = kb_decompose_char(seq.code_point, dst, opts);
   while (!seq.terminal) {
      seq = kb_sequences[++idx];
      nr += kb_decompose_char(seq.code_point, &dst[nr], opts);
   }
   return nr;
}

local size_t kb_decompose_char(char32_t uc,
                               char32_t dst[static KB_MAX_DECOMPOSITION],
                               unsigned options)
{
   kb_assert(kb_code_point_valid(uc));

   const struct kb_property *restrict property = kb_get_property(uc);
   uint32_t category = property->category;

   if (options & (KB_COMPOSE | KB_DECOMPOSE)) {
      size_t ret = kb_decompose_hangul(uc, dst);
      if (ret)
         return ret;
   }
   if ((options & KB_STRIP_IGNORABLE) && property->ignorable)
      return 0;
   if ((options & KB_STRIP_UNKNOWN)) {
      if (category == KB_CATEGORY_CN || category == KB_CATEGORY_CO)
         return 0;
   }
   if (options & KB_LUMP) {
      if (category == KB_CATEGORY_ZS)
         return kb_decompose_lump(0x0020);
      if (category == KB_CATEGORY_PD || uc == 0x2212)
         return kb_decompose_lump(0x002D);
      if (uc == 0x2044 || uc == 0x2215)
         return kb_decompose_lump(0x002F);
      if (uc == 0x2236)
         return kb_decompose_lump(0x003A);
      if (uc == 0x2329 || uc == 0x3008)
         return kb_decompose_lump(0x003C);
      if (uc == 0x232A || uc == 0x3009)
         return kb_decompose_lump(0x003E);
      if (uc == 0x2216) kb_decompose_lump(0x005C);
      if (uc == 0x02C4 || uc == 0x02C6 || uc == 0x2038 || uc == 0x2303)
         return kb_decompose_lump(0x005E);
      if (category == KB_CATEGORY_PC || uc == 0x02CD)
         return kb_decompose_lump(0x005F);
      if (uc == 0x02CB)
         return kb_decompose_lump(0x0060);
   }
   if (options & KB_STRIP_DIACRITIC) {
      if (category == KB_CATEGORY_MN ||
         category == KB_CATEGORY_MC ||
         category == KB_CATEGORY_ME) return 0;
   }
   if (options & KB_CASE_FOLD) {
      if (property->casefold_mapping != UINT16_MAX)
         return kb_decompose_seq(dst, property->casefold_mapping, options);
   }
   if (options & (KB_COMPOSE | KB_DECOMPOSE)) {
      if (property->decomp_mapping != UINT16_MAX && (!property->decomp_type || (options & KB_COMPAT)))
         return kb_decompose_seq(dst, property->decomp_mapping, options);
   }
   *dst = uc;
   return 1;
}

local void kb_canonical_reorder(char32_t *str, size_t len)
{
   size_t i = 0;

   while (i + 1 < len) {
      char32_t uc1 = str[i];
      char32_t uc2 = str[i + 1];
      uint16_t k1 = kb_get_property(uc1)->combining_class;
      uint16_t k2 = kb_get_property(uc2)->combining_class;
      if (k1 > k2 && k2 > 0) {
         str[i] = uc2;
         str[i + 1] = uc1;
         if (i > 0)
            i--;
         else
            i++;
      } else {
         i++;
      }
   }
}

static int kb_decompose(struct kabak *restrict kb,
                        const char *restrict str, size_t len,
                        unsigned options, size_t *restrict lenp)
{
   int ret = KB_OK;
   size_t wpos = 0;

   for (size_t i = 0, clen; i < len; i += clen) {
      char32_t uc = kb_decode_s(&str[i], len - i, &clen);
      if (uc == KB_REPLACEMENT_CHAR && clen == 1)
         ret = KB_EUTF8;

      char32_t *buf = kb_grow(kb, sizeof(char32_t[KB_MAX_DECOMPOSITION]));
      size_t decomp_result = kb_decompose_char(uc, buf, options);
      kb_assert(decomp_result <= KB_MAX_DECOMPOSITION);
      wpos += decomp_result;
      kb->len = sizeof(char32_t[wpos]);
   }
   if (options & (KB_COMPOSE | KB_DECOMPOSE))
      kb_canonical_reorder((char32_t *)kb->str, wpos);
   
   *lenp = wpos;
   return ret;
}

static bool kb_compose_hangul(char32_t *starter, char32_t current_char)
{
   char32_t hangul_lindex = *starter - KB_HANGUL_LBASE;
   if (hangul_lindex < KB_HANGUL_LCOUNT) {
      char32_t hangul_vindex = current_char - KB_HANGUL_VBASE;
      if (hangul_vindex < KB_HANGUL_VCOUNT) {
         *starter = KB_HANGUL_SBASE +
            (hangul_lindex * KB_HANGUL_VCOUNT + hangul_vindex) *
            KB_HANGUL_TCOUNT;
         return true;
      }
   }
   char32_t hangul_sindex = *starter - KB_HANGUL_SBASE;
   if (hangul_sindex < KB_HANGUL_SCOUNT && (hangul_sindex % KB_HANGUL_TCOUNT) == 0) {
      char32_t hangul_tindex = current_char - KB_HANGUL_TBASE;
      if (hangul_tindex < KB_HANGUL_TCOUNT) {
         *starter += hangul_tindex;
         return true;
      }
   }
   return false;
}

local size_t kb_compose(char32_t *buffer, size_t length, unsigned options)
{
   kb_assert(options & KB_COMPOSE);
   
   char32_t *starter = NULL;
   const struct kb_property *starter_property = NULL;
   int32_t max_combining_class = -1;
   size_t wpos = 0;

   for (size_t rpos = 0; rpos < length; rpos++) {
      char32_t current_char = buffer[rpos];
      const struct kb_property *current_property = kb_get_property(current_char);
      if (starter && current_property->combining_class > max_combining_class) {
         /* combination perhaps possible */
         if (kb_compose_hangul(starter, current_char)) {
            starter_property = NULL;
            continue;
         }
         if (!starter_property) {
            starter_property = kb_get_property(*starter);
         }
         if (starter_property->comb1st_index >= 0 &&
             current_property->comb2nd_index >= 0) {
            char32_t composition = kb_combinations[
               starter_property->comb1st_index +
               current_property->comb2nd_index
            ];
            if (composition != KB_BAD_CHAR && !kb_get_property(composition)->comp_exclusion) {
               *starter = composition;
               starter_property = NULL;
               continue;
            }
         }
      }
      buffer[wpos] = current_char;
      if (current_property->combining_class) {
         if (current_property->combining_class > max_combining_class) {
            max_combining_class = current_property->combining_class;
         }
      } else {
         starter = buffer + wpos;
         starter_property = NULL;
         max_combining_class = -1;
      }
      wpos++;
   }
   return wpos;
}

int kb_transform(struct kabak *restrict kb, const char *restrict str,
                 size_t len, unsigned opts)
{
   kb_clear(kb);

   int ret = kb_decompose(kb, str, len, opts, &len);
   if (len) {
      void *restrict ustr = kb->str;
      if (opts & KB_COMPOSE)
         len = kb_compose(ustr, len, opts);
      kb->len = kb_encode_inplace(ustr, len);
   }
   return ret;
}
