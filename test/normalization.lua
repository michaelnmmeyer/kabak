#!/usr/bin/env lua5.3

local kabak = require("kabak")

local function seq_to_str(seq)
   local cps = {}
   for cp in seq:gmatch("%x+") do
      local n = assert(tonumber("0x" .. cp))
      table.insert(cps, n)
   end
   return utf8.char(table.unpack(cps))
end

local function str_to_seq(str)
   local cps = {}
   for _, cp in utf8.codes(str) do
      table.insert(cps, string.format("%04X", cp))
   end
   return table.concat(cps, " ")
end

local longest_decomposition = 0

local function transform(...)
   local ret = kabak.transform(...)
   local len = utf8.len(ret)
   if len > longest_decomposition then
      longest_decomposition = len
   end
   return ret
end

local function exec_nfc(x, y)
   local ret = transform(x)
   if ret ~= y then error("fail") end
end

local function exec_merge(x, y)
   local z = transform(x, kabak.MERGE)
   if z ~= y then
      local xs = str_to_seq(x)
      local ys = str_to_seq(y)
      local zs = str_to_seq(z)
      print(string.format("%s (%s) -> %s (%s) != %s (%s)", x, xs, z, zs, y, ys))
   end
end

-- NFC
--   c2 ==  toNFC(c1) ==  toNFC(c2) ==  toNFC(c3)
--   c4 ==  toNFC(c4) ==  toNFC(c5)
local function check_nfc(fs)
   exec_nfc(fs[1], fs[2])
   exec_nfc(fs[2], fs[2])
   exec_nfc(fs[3], fs[2])
   exec_nfc(fs[4], fs[4])
   exec_nfc(fs[5], fs[4])
end

-- NFKC
--   c4 == toNFKC(c1) == toNFKC(c2) == toNFKC(c3) == toNFKC(c4) == toNFKC(c5)
local function check_merge(fs)
   for i = 1, 5 do exec_merge(fs[i], fs[4]) end
end

local tests = {check_nfc, check_merge}

local part_no = -1
local changing = {}   -- Code points in part 1.

for line in io.lines() do
   local c = line:sub(1, 1)
   if c == "@" then
      part_no = part_no + 1
   elseif c ~= "#" and c ~= "" then
      local fields = {line = line}
      for seq in line:gmatch("(%x%x%x%x[%s%x]*);") do
         table.insert(fields, seq_to_str(seq))
      end
      assert(#fields == 5)
      for _, test in ipairs(tests) do
         test(fields)
      end
      if part_no == 1 then
         local c = fields[1]
         changing[c] = true
      end
   end
end

-- Skip surrogates.
local ranges = {{0, 0xD800 - 1}, {0xDFFF + 1, 0x10FFFF}}

for _, range in ipairs(ranges) do
   for i = range[1], range[2] do
      local c = utf8.char(i)
      if not changing[c] then
         exec_nfc(c, c)
         exec_merge(c, c)
      end
   end
end

print("longest decomposition:", longest_decomposition)
