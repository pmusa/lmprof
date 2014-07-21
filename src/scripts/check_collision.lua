local REPORT = dofile(... or "lmprof_default_output.txt")

local h = {}
for _,v in pairs(REPORT) do
  local key = bit32.bxor(v.func, v.parent) % 17
  h[key] = h[key] and h[key] + 1 or 1
end

for i=0,16 do
  print(h[i])
end
