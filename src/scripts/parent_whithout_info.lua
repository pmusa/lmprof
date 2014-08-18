if not arg[1] then
  print("usage: " .. arg[0] .. " <lmprof_output_file_location>")
  os.exit(1)
end

local prof_filename = arg[1]
local ok, call_table = pcall(dofile, prof_filename)
if not ok then
  print(call_table)
  os.exit(1)
end


local p = {}
local f = {}
for _, v in pairs(call_table) do
  f[v.func] = true
  p[v.parent] = true
end

print("Parents that lmprof does not have information about:")
local s = ""
for k,_ in pairs(p) do
  if not f[k] then
    s = s .. string.format('"%s", ', k) 
  end
end

print(s)

