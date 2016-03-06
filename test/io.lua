local line_test = {
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
   
   "", "",
   
   -- Strip ignorable characters, e.g. SOFT HYPHEN.
   "\xc2\xad", "",
   "\xc2\xad\nfoo\n", "\nfoo\n",
}

local para_test = {
   "foo\n\nbar", "<<foo\n>><<bar\n>>",
   "foo\r\n\r\nbar", "<<foo\n>><<bar\n>>",
   "foo\r\nbar", "<<foo\nbar\n>>",
   "foo\xe2\x80\xa9bar", "<<foo\n>><<bar\n>>",
   "\n\nfoo\n\n\n\nbar\n\n\n\n", "<<foo\n>><<bar\n>>",
}

local function do_test(tbl, mode)
   local path = "io.tmp"
   for i = 1, #tbl, 2 do
      local fp = assert(io.open(path, "wb"))
      fp:write(tbl[i])
      fp:close()
      local cmd = string.format("./io %s %s", path, mode)
      fp = assert(io.popen(cmd))
      local ret = fp:read("*a")
      fp:close()
      assert(ret == tbl[i + 1], i, ret)
   end
   os.remove(path)
end

do_test(line_test, "line")
do_test(para_test, "para")
