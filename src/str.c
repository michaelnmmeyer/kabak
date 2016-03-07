#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdnoreturn.h>
#include "api.h"
#include "imp.h"

local void (*kb_error_handler)(const char *);

local noreturn void kb_oom(void)
{
   if (kb_error_handler)
      kb_error_handler("out of memory");
   abort();
}

void kb_on_error(void (*handler)(const char *))
{
   kb_error_handler = handler;
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

void kb_truncate(struct kabak *kb, size_t len)
{
   kb_assert(len == 0 || len < kb->alloc);
   if (kb->alloc)
      kb->str[kb->len = len] = '\0';
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
   kb->len += len;
}

void kb_catc(struct kabak *restrict kb, char32_t c)
{
   char *restrict buf = kb_grow(kb, 4);
   size_t clen = kb_encode(buf, c);
   buf[clen] = '\0';
   kb->len += clen;
}

void kb_catb(struct kabak *restrict kb, int c)
{
   kb_assert(c >= 0 && c <= 0xff);
   char *restrict buf = kb_grow(kb, 1);
   *buf++ = c;
   *buf = '\0';
   kb->len++;
}

static size_t kb_vsnprintf_unsigned(char *restrict buf, size_t size,
                                    const char *restrict fmt, va_list ap)
{
   int len = vsnprintf(buf, size, fmt, ap);
   return len < 0 ? 0 : len;
}

static void kb_vprintf(struct kabak *restrict kb, const char *restrict fmt,
                       va_list ap)
{
   va_list copy;
   va_copy(copy, ap);
   size_t avail = kb->alloc - kb->len;
   size_t size = kb_vsnprintf_unsigned(&kb->str[kb->len], avail, fmt, copy);
   va_end(copy);

   if (size >= avail) {
      char *restrict buf = kb_grow(kb, size);
      avail = kb->alloc - kb->len;
      size = kb_vsnprintf_unsigned(buf, avail, fmt, ap);
   }
   kb->len += size;
}

void kb_printf(struct kabak *restrict kb, const char *restrict fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   kb_vprintf(kb, fmt, ap);
   va_end(ap);
}

char *kb_detach(struct kabak *restrict kb, size_t *restrict len)
{
   if (kb->alloc) {
      if (len)
         *len = kb->len;
      return kb->str;
   }
   char *ret = calloc(1, 1);
   if (!ret)
      kb_oom();
   if (len)
      *len = 0;
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
