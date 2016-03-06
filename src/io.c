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

enum {
   LINE_FEED = '\n',
   CARRIAGE_RETURN = '\r',
   VERTICAL_TAB = 0x000B,
   FORM_FEED = 0x000C,
   NEXT_LINE = 0x0085,
   LINE_SEPARATOR = 0x2028,
   PARAGRAPH_SEPARATOR = 0x2029,
};

local bool kb_break_line(struct kb_file *fp, char32_t c)
{
   size_t clen;
   switch (c) {
   case LINE_FEED:
   case VERTICAL_TAB:
   case FORM_FEED:
   case NEXT_LINE:
   case LINE_SEPARATOR:
   case PARAGRAPH_SEPARATOR:
      return true;
   case CARRIAGE_RETURN:
      c = kb_getc(fp, &clen);
      if (c != LINE_FEED)
         kb_ungetc(c, fp);
      return true;
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
                        size_t *restrict len, char32_t *restrict cp)
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
   if (cp)
      *cp = c;
   
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
   int ret = kb_fdecompose(kb, fp, opts, &len, NULL);
   if (len) {
      void *restrict ustr = kb->str;
      if (opts & KB_COMPOSE)
         len = kb_compose(ustr, len, opts);
      kb->len = kb_encode_inplace(ustr, len);
   }
   return ret;
}

static bool kb_all_whitespace(const char32_t *str, size_t len)
{
   for (size_t i = 0; i < len; i++)
      if (!kb_is_space(str[i]))
         return false;
   return true;
}

int kb_get_para(struct kb_file *restrict fp, struct kabak *restrict kb,
                unsigned opts)
{
   kb_clear(kb);

   struct kabak line = KB_INIT;
   bool have_text = false;
   int ret = KB_OK;

   do {
      size_t len;
      char32_t c;
      kb_clear(&line);
      ret = kb_fdecompose(&line, fp, opts, &len, &c);
      if (kb_all_whitespace((char32_t *)line.str, len)) {
         if (have_text)
            break;
      } else {
         kb_reencode(&line, len, opts);
         kb_cat(kb, line.str, line.len);
         kb_catb(kb, '\n');
         have_text = true;
      }
      if (c == PARAGRAPH_SEPARATOR && have_text)
         break;
   } while (ret != KB_FINI);

   kb_fini(&line);
   if (ret == KB_FINI && kb->len)
      return KB_OK;
   return ret;
}
