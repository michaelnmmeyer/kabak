#include "api.h"
#include "imp.h"

local int kb_getb(struct kb_file *fp)
{
   return fp->pending ? fp->backup[--fp->pending] : getc(fp->fp);
}

local void kb_ungetbs(struct kb_file *restrict fp, const uint8_t *restrict cs,
                      size_t nr)
{
   kb_assert(!fp->pending);

   while (nr)
      fp->backup[--nr] = cs[fp->pending++];
}

local void kb_skip_bom(struct kb_file *fp)
{
   static const uint8_t bom[] = {0xEF, 0xBB, 0xBF};

   for (size_t i = 0; i < sizeof bom / sizeof *bom; i++) {
      int b = getc(fp->fp);
      if (b != bom[i]) {
         ungetc(b, fp->fp);
         kb_ungetbs(fp, bom, i);
         break;
      }
   }
}

local void kb_ungetc(char32_t c, struct kb_file *fp)
{
   fp->last = c;
}

local char32_t kb_getc(struct kb_file *restrict fp, size_t *restrict clen)
{
   char32_t c = fp->last;
   if (c != KB_BAD_CHAR) {
      fp->last = KB_BAD_CHAR;
      return c;
   }

   int b = kb_getb(fp);
   if (b == EOF) {
      *clen = 0;
      return KB_EOF;
   }

   size_t len = kb_utf8class[b];
   if (!len)
      goto bad;

   uint8_t buf[4];
   buf[0] = b;
   for (size_t i = 1; i < len; i++) {
      b = kb_getb(fp);
      if (b == EOF) {
         kb_ungetbs(fp, &buf[1], i - 1);
         goto bad;
      }
      buf[i] = b;
   }

   c = kb_sdecode(buf, len);
   if (c == KB_BAD_CHAR) {
      kb_ungetbs(fp, &buf[1], len - 1);
      goto bad;
   }
   *clen = len;
   return c;

bad:
   *clen = 1;
   return KB_REPLACEMENT_CHAR;
}

local bool kb_break_line(struct kb_file *fp, char32_t c)
{
   size_t clen;

   switch (c) {
   case 0x000A:   /* Line feed. */
   case 0x000B:   /* Vertical tab. */
   case 0x000C:   /* Form feed. */
   case 0x0085:   /* Next Line. */
   case 0x2028:   /* Line separator. */
   case 0x2029:   /* Paragraph Separator. */
      return true;
   case 0x000D: {   /* Carriage return. */
      c = kb_getc(fp, &clen);
      if (c != '\n')
         kb_ungetc(c, fp);
      return true;
   }
   default:
      return false;
   }
}

local bool kb_at_end(struct kb_file *restrict fp, struct kabak *restrict buf)
{
   if (buf->len)
      return false;
   
   size_t clen;
   char32_t c = kb_getc(fp, &clen);
   kb_ungetc(c, fp);
   return c == KB_EOF;
}

void kb_wrap(struct kb_file *restrict fp, FILE *restrict xfp)
{
   fp->fp = xfp;
   fp->pending = 0;
   fp->last = KB_BAD_CHAR;
   
   kb_skip_bom(fp);
}

local int kb_fdecompose(struct kabak *restrict kb,
                        struct kb_file *restrict fp, unsigned opts,
                        size_t *len)
{
   int ret = KB_OK;
   char32_t c;
   size_t clen;
   while ((c = kb_getc(fp, &clen)) != KB_EOF && !kb_break_line(fp, c)) {
      if (c == KB_REPLACEMENT_CHAR && clen == 1)
         ret = KB_EUTF8;

      char32_t *buf = kb_grow(kb, sizeof(char32_t[KB_MAX_DECOMPOSITION]));
      clen = kb_decompose_char(c, buf, opts);
      kb->len += sizeof(char32_t[clen]);
   }
   
   *len = kb->len / sizeof(char32_t);
   kb_canonical_reorder((char32_t *)kb->str, *len);

   if (ret == KB_OK && kb_at_end(fp, kb))
      return KB_FINI;
   return ret;
}

int kb_get_line(struct kb_file *restrict fp, struct kabak *restrict kb,
                unsigned opts)
{
   kb_clear(kb);

   size_t len;
   int ret = kb_fdecompose(kb, fp, opts, &len);

   void *restrict ustr = kb->str;
   if (opts & KB_COMPOSE)
      len = kb_compose(ustr, len, opts);

   if (len)
      kb->len = kb_encode_inplace(ustr, len);
   return ret;
}
