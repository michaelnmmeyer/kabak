#include "../kabak.h"
#include <stdlib.h>
#include <string.h>

static const unsigned opts = KB_NFC | KB_STRIP_IGNORABLE | KB_STRIP_UNKNOWN;

int main(int argc, char **argv)
{
   if (argc != 3)
      abort();
   
   const char *path = argv[1];
   const char *mode = argv[2];

   FILE *f = fopen(path, "rb");
   if (!f)
      abort();

   struct kb_file fp;
   kb_wrap(&fp, f);

   int ret;
   struct kabak buf = KB_INIT;
   if (!strcmp(mode, "line")) {
      while ((ret = kb_get_line(&fp, &buf, opts)) != KB_FINI)
         puts(buf.str);
   } else if (!strcmp(mode, "para")) {
      while ((ret = kb_get_para(&fp, &buf, opts)) != KB_FINI)
         printf("<<%s>>", buf.str);
   } else {
      abort();
   }
   kb_fini(&buf);
   
   fclose(f);
}
