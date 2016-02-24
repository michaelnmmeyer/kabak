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
   while ((ret = kb_get_line(&fp, &buf, KB_XNFC)) != KB_FINI)
      puts(buf.str);
   
   fclose(f);
}
