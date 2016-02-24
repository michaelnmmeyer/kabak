# kabak

Minimalistic Unicode support for modern C.

## Purpose

This is an experimental Unicode processing library. It derives from the
[utf8proc library](https://github.com/JuliaLang/utf8proc). The following common
operations are supported :

* Normalize a string to NFC.
* Read a file line by line, transparently performing NFC normalization, taking
  into account all possible Unicode EOL terminators.
* UTF-8 encoding and decoding, computation of code points offsets in UTF-8
  encoded strings.
* Normalization to NFKC, with additional custom mappings, to simplify strings
  representation for text search and text classification programs.
* Case folding.
* Diacritic removal.

## Policy

Dealing with Unicode is a constant chore, even more so in C. To keep one's
sanity, the following should always be performed, in order :

1. Convert strings read from files, etc., to UTF-8, if necessary.
2. Normalize the result to NFC.

From this point on, it is guaranteed that each string is valid UTF-8 _and_ can
be meaningfully compared to another string. In effect, Unicode normalization
forms are like secondary encodings, and must be dealt with properly, as is the
case for UTF-8. Anything done before both of the above are done is meaningless.

The following must be kept in mind:
* Unicode strings are normalized to NFC when reading data from a file. This
  behaviour can be undesirable or unnecessary for some niche applications. It
  is, however, required for programs that perform string comparisons -- which is
  to say, most of them --, because two strings cannot be correctly compared for
  equality before they are both normalized to the same form. Many people don't
  know that this is necessary or forget to do it, hence the automatic
  normalization.
* The standard `char32_t` type is used to represent code points. Many people
  define their own type for this purpose. This leads to havoc and
  misunderstandings when some of them use 16 bits integer to represent what they
  call a "character" (Java, Qt, `wchar_t` on Windows), while others don't use a
  fixed-width type (git), and still others use signed integers.
* Once strings are normalized, unsafe functions are used for UTF-8 encoding and
  decoding.
* Use of dynamic memory allocation. There is no good reason to use
  stack-allocated buffers and complicate everything in an attempt to improve
  performance.
