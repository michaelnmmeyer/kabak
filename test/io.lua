local files = {
   -- Skip the leading BOM, but backup if there isn't a full match.
   "\xEF\xBB\xBFabc\n", "abc\n",
   "\xEF\xBBabc\n", "��abc\n",
   "\xEFabc\n", "�abc\n",
   
   -- Replace invalid code points.
   "foo\xffbar", "foo�bar\n",
   
   -- Read full code points.
   "北京\n", "北京\n",
   "éé\nवृत्ति\nèè\n", "éé\nवृत्ति\nèè\n",

   -- Varied NL code points.
   "foo\nbar\r\nbaz\rqux\xe2\x80\xa8", "foo\nbar\nbaz\nqux\n",

   -- Empty lines should not be stripped, except the last one, if empty.
   "\n\n\n", "\n\n",
   "\n\n\n ", "\n\n\n \n",

   -- Read the last line even if there is not a terminal NL character.
   "foo", "foo\n",
}

local path = "io.tmp"

for i = 1, #files, 2 do
   local fp = assert(io.open(path, "wb"))
   fp:write(files[i])
   fp:close()
   local cmd = string.format("./io %s", path)
   fp = assert(io.popen(cmd))
   local ret = fp:read("*a")
   fp:close()
   assert(ret == files[i + 1], i)
end

os.remove(path)
