#include <stdio.h>
#include <stdlib.h>
#include "../kabak.h"
#include "cmd.h"

noreturn static void version(void)
{
   const char *msg =
   "Kabak version "KB_VERSION"\n"
   "Copyright (c) 2016 Michaël Meyer"
   ;
   puts(msg);
   exit(EXIT_SUCCESS);
}

static unsigned options = KB_XNFC;

static void process(const char *path)
{
   FILE *fp = stdin;
   if (path) {
      fp = fopen(path, "r");
      if (!fp)
         die("cannot open file:");
   }
   
   struct kabak data = KB_INIT;
   size_t len;
   do {
      kb_grow(&data, BUFSIZ);
      len = fread(&data.str[data.len], 1, BUFSIZ, fp);
      data.len += len;
   } while (len);
   
   if (ferror(fp))
      die("cannot read file:");
   
   struct kabak norm = KB_INIT;
   int ret = kb_transform(&norm, data.str, data.len, options);
   if (ret)
      die("cannot normalize file: %s", kb_strerror(ret));
   
   if (fwrite(norm.str, 1, norm.len, stdout) != norm.len)
      die("I/O error:");
}

static void case_fold(void) { options |= KB_XCASE_FOLD; }
static void diacr_fold(void) { options |= KB_XSTRIP_DIACRITIC; }
static void merge(void) { options |= KB_XNFKC; }

int main(int argc, char **argv)
{
   struct option opts[] = {
      {'c', "case-fold", OPT_FUNC(case_fold)},
      {'d', "strip-diacritic", OPT_FUNC(diacr_fold)},
      {'m', "merge", OPT_FUNC(merge)},
      {'\0', "version", OPT_FUNC(version)},
      {0},
   };
   const char help[] =
      #include "kabak.ih"
   ;

   parse_options(opts, help, &argc, &argv);
   if (argc > 1)
      die("excess arguments");
   
   process(*argv);
}
