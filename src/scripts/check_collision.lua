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

local BUCKETS = 97
local max = 0
local min = 999
local empty = 0
local total = 0
local h = {}
for _,v in pairs(call_table) do
  local key = bit32.bxor(v.func, v.parent) % BUCKETS
  h[key] = h[key] and h[key] + 1 or 1
end

for i=0,(BUCKETS - 1) do
  if h[i] then
    if h[i] > max then max = h[i] end
    if h[i] < min then min = h[i] end
    total = total + h[i]
  else
    empty = empty + 1
  end
end

local av = total/BUCKETS
local gtm = 0
local gta = 0
for i=0,(BUCKETS - 1) do
  if h[i] and h[i] > max/2 then gtm = gtm + 1 end
  if h[i] and h[i] > av then gta = gta + 1 end
end

print("total : " .. total)
print("max colisions: " .. max)
print("min colisions: " .. min)
print("empty buckets: " .. empty)
print("greater than median (" .. max/2 .. "): " .. gtm)
print(string.format("greater than average (%.1f): %d", av, gta))
