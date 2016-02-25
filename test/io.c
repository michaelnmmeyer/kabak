#include "../kabak.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
   struct kabak buf = KB_INIT;
   struct kb_file fp;
   int ret;
   
   if (argc != 2)
      abort();

   FILE *f = fopen(argv[1], "rb");
   if (!f)
      abort();

   kb_wrap(&fp, f);
   const unsigned opts = KB_NFC | KB_STRIP_IGNORABLE | KB_STRIP_UNKNOWN;
   while ((ret = kb_get_line(&fp, &buf, opts)) != KB_FINI)
      puts(buf.str);
   
   fclose(f);
}
