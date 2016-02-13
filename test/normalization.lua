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

local function check(fs, x, y, opts)
   local ret = kabak.transform(fs[y], opts)
   if fs[x] ~= ret then
      print(fs.line)
      print(string.format("%d %d -> %s != %s", x, y, ret, fs[x]))
      error("fail")
   end
end

-- NFC
--   c2 ==  toNFC(c1) ==  toNFC(c2) ==  toNFC(c3)
--   c4 ==  toNFC(c4) ==  toNFC(c5)
local function check_nfc(fs)
   check(fs, 2, 1)
   check(fs, 2, 2)
   check(fs, 2, 3)
   check(fs, 4, 4)
   check(fs, 4, 5)
end

local tests = {check_nfc}

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
         assert(kabak.transform(c) == c, c)
      end
   end
end
