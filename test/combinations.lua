#!/usr/bin/env lua5.3

-- Computes the length of the longest possible decomposition, given our custom
-- mappings, for all possible options combinations. Must be run under Valgrind
-- to detect overreads.

local kb = require("kabak")

local longest_decomposition = 0

local norm_modes = {
   kb.MERGE,
   kb.CASE_FOLD,
   kb.DIACR_FOLD,
   kb.MERGE + kb.CASE_FOLD,
   kb.MERGE + kb.DIACR_FOLD,
   kb.CASE_FOLD + kb.DIACR_FOLD,
   kb.MERGE + kb.CASE_FOLD + kb.DIACR_FOLD,
}

local ranges = {{0, 0xD800 - 1}, {0xDFFF + 1, 0x10FFFF}}

for _, opts in ipairs(norm_modes) do
   for _, range in ipairs(ranges) do
      for i = range[1], range[2] do
         local c = utf8.char(i)
         local ret = kb.transform(c, opts)
         local len = utf8.len(ret)
         if len > longest_decomposition then
            longest_decomposition = len
         end
      end
   end
end

print("longest_decomposition", longest_decomposition)
