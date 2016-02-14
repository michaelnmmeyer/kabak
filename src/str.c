#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdnoreturn.h>
#include "api.h"
#include "imp.h"

local noreturn void kb_oom(void)
{
   fprintf(stderr, "kabak: out of memory");
   abort();
}

void kb_fini(struct kabak *kb)
{
   if (kb->alloc)
      free(kb->str);
}

void kb_clear(struct kabak *kb)
{
   if (kb->alloc)
      kb->str[kb->len = 0] = '\0';
}

#define KB_INIT_SIZE sizeof(char32_t[KB_MAX_DECOMPOSITION])

void *kb_grow(struct kabak *kb, size_t size)
{
   size_t need = kb->len + size + 1;
   if (need <= kb->len)
      kb_oom();

   if (need > kb->alloc) {
      if (kb->alloc) {
         kb->alloc += kb->alloc >> 1;
         if (kb->alloc < need)
            kb->alloc = need;
         kb->str = realloc(kb->str, kb->alloc);
      } else {
         kb->alloc = need < KB_INIT_SIZE ? KB_INIT_SIZE : need;
         kb->str = malloc(kb->alloc);
      }
   }
   if (!kb->str)
      kb_oom();
   return &kb->str[kb->len];
}

void kb_cat(struct kabak *restrict kb, const char *restrict str, size_t len)
{
   char *restrict buf = kb_grow(kb, len);
   memcpy(buf, str, len);
   buf[len] = '\0';
}

void kb_catc(struct kabak *restrict kb, char32_t c)
{
   char *restrict buf = kb_grow(kb, 4);
   const size_t clen = kb_encode(buf, c);
   buf[clen] = '\0';
   kb->len += clen;
}

char *kb_detach(struct kabak *restrict kb, size_t *restrict len)
{
   if (kb->alloc) {
      *len = kb->len;
      return kb->str;
   }
   *len = 0;
   char *ret = calloc(1, 1);
   if (!ret)
      kb_oom();
   return ret;
}

const char *kb_strerror(int err)
{
   static const char *const tbl[] = {
      [KB_OK] = "no error",
      [KB_FINI] = "end of iteration",
      [KB_EUTF8] = "invalid UTF-8 sequence",
   };
   
   if (err >= 0 && (size_t)err < sizeof tbl / sizeof *tbl)
      return tbl[err];
   return "unknown error";
}
