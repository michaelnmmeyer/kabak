#include <assert.h>
#include <stdio.h>
#include "api.h"
#include "imp.h"

static const uint8_t kb_utf8class[256] = {
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
   3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
   4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0,
};

static char32_t kb_sdecode(const uint8_t *str, size_t clen)
{
   char32_t uc;

   switch (clen) {
   case 1:
      uc = str[0];
      break;
   case 2:
      if ((str[1] & 0xC0) != 0x80)
         goto bad;
      uc = ((str[0] & 0x1F) << 6) + (str[1] & 0x3F);
      if (uc < 0x80)
         goto bad;
      break;
   case 3:
      if ((str[1] & 0xC0) != 0x80 || (str[2] & 0xC0) != 0x80)
         goto bad;
      uc = ((str[0] & 0x0F) << 12) + ((str[1] & 0x3F) << 6) + (str[2] & 0x3F);
      if (uc < 0x800)
         goto bad;
      if (uc >= 0xD800 && uc <= 0xDFFF)
         goto bad;
      break;
   default:
      if ((str[1] & 0xC0) != 0x80 || (str[2] & 0xC0) != 0x80 || (str[3] & 0xC0) != 0x80)
         goto bad;
      uc = ((str[0] & 0x07) << 18) + ((str[1] & 0x3F) << 12)
         + ((str[2] & 0x3F) << 6) + (str[3] & 0x3F);
      if (uc < 0x10000 || uc >= 0x110000)
         goto bad;
      break;
   }
   return uc;

bad:
   return KB_BAD_CHAR;
}

local char32_t kb_rnext(const uint8_t *restrict str, size_t len,
                        size_t *restrict clen)
{
   if (!len) {
      *clen = 0;
      return KB_REPLACEMENT_CHAR;
   }

   *clen = kb_utf8class[*str];
   if (!*clen || *clen > len)
      goto bad;
   
   char32_t c = kb_sdecode(str, *clen);
   if (c == KB_BAD_CHAR)
      goto bad;

   return c;

bad:
   *clen = 1;
   return KB_REPLACEMENT_CHAR;
}

char32_t kb_decode(const char *restrict str, size_t *restrict clen)
{
   *clen = kb_utf8class[(uint8_t)*str];
   kb_assert(*clen > 0);

   char32_t c;
   switch (*clen) {
   case 1:
      c = (uint8_t)str[0];
      break;
   case 2:
      c = (((uint8_t)str[0] & 0x1F) << 6) + ((uint8_t)str[1] & 0x3F);
      break;
   case 3:
      c = (((uint8_t)str[0] & 0x0F) << 12) + (((uint8_t)str[1] & 0x3F) << 6)
             + ((uint8_t)str[2] & 0x3F);
      break;
   default:
      c = (((uint8_t)str[0] & 0x07) << 18) + (((uint8_t)str[1] & 0x3F) << 12)
              + (((uint8_t)str[2] & 0x3F) << 6) + ((uint8_t)str[3] & 0x3F);
      break;
   }
   
   kb_assert(kb_code_point_valid(c));
   return c;
}

size_t kb_encode(char dst[static 4], char32_t uc)
{
   kb_assert(kb_code_point_valid(uc));

   if (uc < 0x80) {
      dst[0] = uc;
      return 1;
   }
   if (uc < 0x800) {
      dst[0] = 0xC0 + (uc >> 6);
      dst[1] = 0x80 + (uc & 0x3F);
      return 2;
   }
   if (uc < 0x10000) {
      dst[0] = 0xE0 + (uc >> 12);
      dst[1] = 0x80 + ((uc >> 6) & 0x3F);
      dst[2] = 0x80 + (uc & 0x3F);
      return 3;
   }
   dst[0] = 0xF0 + (uc >> 18);
   dst[1] = 0x80 + ((uc >> 12) & 0x3F);
   dst[2] = 0x80 + ((uc >> 6) & 0x3F);
   dst[3] = 0x80 + (uc & 0x3F);
   return 4;
}

local size_t kb_encode_inplace(char32_t *str, size_t len)
{
   size_t new_len = 0;

   for (size_t i = 0; i < len; i++) {
      char32_t c = str[i];
      new_len += kb_encode(((char *)str) + new_len, c);
   }
   ((char *)str)[new_len] = '\0';
   return new_len;
}

size_t kb_count(const char *str, size_t len)
{
   size_t nr = 0;
   
   for (size_t i = 0, clen; i < len; i += clen) {
      clen = kb_utf8class[(uint8_t)str[i]];
      kb_assert(clen > 0 && i + clen <= len);
      nr++;
   }
   return nr;
}
