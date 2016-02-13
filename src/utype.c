#include <assert.h>
#include "imp.h"

enum kb_category kb_category(char32_t c)
{
   kb_assert(kb_code_point_valid(c));

   return kb_get_property(c)->category;
}

static uint32_t kb_category_mask(char32_t c)
{
   return 1 << kb_category(c);
}

bool kb_is_letter(char32_t c)
{
   kb_assert(kb_code_point_valid(c));

   const uint32_t mask = (1 << KB_CATEGORY_LU)
                       | (1 << KB_CATEGORY_LL)
                       | (1 << KB_CATEGORY_LT)
                       | (1 << KB_CATEGORY_LM)
                       | (1 << KB_CATEGORY_LO)
                       ;
   return kb_category_mask(c) & mask;
}

bool kb_is_upper(char32_t c)
{
   kb_assert(kb_code_point_valid(c));

   return kb_category(c) == KB_CATEGORY_LU;
}

bool kb_is_lower(char32_t c)
{
   kb_assert(kb_code_point_valid(c));

   return kb_category(c) == KB_CATEGORY_LL;
}

bool kb_is_number(char32_t c)
{
   kb_assert(kb_code_point_valid(c));

   const uint32_t mask = (1 << KB_CATEGORY_ND)
                       | (1 << KB_CATEGORY_NL)
                       | (1 << KB_CATEGORY_NO)
                       ;
   return kb_category_mask(c) & mask;
}

/* Special cases are:
 *   0009..000D    ; White_Space # Cc   [5] <control-0009>..<control-000D>
 *   0085          ; White_Space # Cc       <control-0085>
 * See http://www.unicode.org/Public/8.0.0/ucd/PropList.txt 
 */
bool kb_is_space(char32_t c)
{
   kb_assert(kb_code_point_valid(c));

   if (c == 0x0020 || (c >= 0x0009 && c <= 0x000d) || c == 0x0085)
      return true;

   const uint32_t kmask = (1 << KB_CATEGORY_ZS)
                        | (1 << KB_CATEGORY_ZL)
                        | (1 << KB_CATEGORY_ZP)
                        ;
   return kb_category_mask(c) & kmask;
}
