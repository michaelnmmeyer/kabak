#!/usr/bin/env python3

"""
Adds decomposition mappings for some characters to the Unicode database, and
prints the result.

To do things the right way, we should handle these characters separately, but it
is simpler to put everything in the same place. We need to use NKFC
normalization anyway, so why not trying to stuff everything there?

Previously, we had a special function to fold ligatures to ASCII, but this
was clunky at best. Besides, there is no reason to not fold them since other
ligatures (ĳ, Ĳ, ﬂ, ﬀ, ﬃ, ﬄ, ﬅ) are systematically folded.

The only issue is that we can't do true NFKC normalization anymore, but this is
not a problem for now.
"""

import sys

REPLACEMENTS = eval(open("lump.py").read())

# Example:
# FB03;LATIN SMALL LIGATURE FFI;Ll;0;L;<compat> 0066 0066 0069;;;;N;;;;;

HEX_FIELD_NO = 0
COMPAT_FIELD_NO = 5

def char_to_hex(cs):
   return " ".join("%04x" % ord(c) for c in cs)

def hex_to_char(h):
   return chr(int(h, 16))

num_replaced = 0

for line in sys.stdin:
   line = line.strip()
   fields = line.split(";")
   char = hex_to_char(fields[HEX_FIELD_NO])
   replacement = REPLACEMENTS.get(char)
   if replacement:
      fields[COMPAT_FIELD_NO] = "<compat> %s" % char_to_hex(replacement)
      num_replaced += 1
   print(";".join(fields))

assert num_replaced == len(REPLACEMENTS)
