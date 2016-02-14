#include "../kabak.h"
#include <string.h>
#include <stdlib.h>

static const struct {
   const char *s;
   ptrdiff_t nr;
   size_t ret;
} tests[] = {
   {"", 0, 0},
   {"é", 0, 0},
   {"é", 1, 2},
   {"é", -1, 0},
   {"aé", 1, 1},
   {"aé", -1, 1},
   {"aé", 333, 3},
   {"aé", -333, 0},
};

int main(void)
{
   for (size_t i = 0; i < sizeof tests / sizeof *tests; i++) {
      const char *str = tests[i].s;
      size_t len = strlen(str);
      
      char tmp[333];
      memcpy(tmp, str, len);
      size_t ret = kb_offset(str, len, tests[i].nr);
      if (ret != tests[i].ret)
         abort();
   }
}
