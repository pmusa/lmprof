local REPORT = dofile(... or "lmprof_default_output.txt")


local p = {}
local f = {}
for _, v in pairs(REPORT) do
  f[v.func] = true
  p[v.parent] = true
end

for k,_ in pairs(p) do
  if not f[k] then
    print(k)
  end
end

