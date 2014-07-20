local REPORT = dofile(... or "lmprof_default_output.txt")

local ts = 0
local tc = 0
local FUNCS = {}
local IFUNCS = {}

for _, f in pairs(REPORT) do
  tc = tc + f.count
  ts = ts + f.size

  local ft = FUNCS[f.func] or {name = f.name, count = 0, size = 0, parents = {}}
  if not FUNCS[f.func] then
    FUNCS[f.func] = ft
  end

  ft.count = ft.count + f.count
  ft.size = ft.size + f.size

  local pt = ft.parents[f.parent] or {count = 0, size = 0}
  if not ft.parents[f.parent] then
    ft.parents[f.parent] = pt
  end
  pt.count = pt.count + f.count
  pt.size = pt.size + f.size
end

for _, f in pairs(FUNCS) do
  if f.size > 0 then
    table.insert(IFUNCS, f)
  end
end

table.sort(IFUNCS, function(a,b) return a.size > b.size end)

local X = 10
print(#IFUNCS .. " functions allocated data")
print(tc .. " calls allocated data")
print(string.format("%dMB were allocated", ts/1024/1024))
print("\n")
print("Top "..X.." allocations follows:")
print("------------------------------------------------------------")
local pat = "%s - %d - %dMB"
for i=1,X do
  local f = IFUNCS[i]
  print(string.format(pat, f.name, f.count, (f.size/1024/1024)))
end
print("------------------------------------------------------------")

local function format(parc, total)
  return string.format("%d (%d%%)", parc, parc/total*100)
end
for i=1,X do
  local f = IFUNCS[i]
         ----------------------
  print("----------------------------------------------------------")
  if f.parents then
    local ok = false
    for k,v in pairs(f.parents) do
      ok = true
      if FUNCS[k] then
        print(string.format("\t  PARENT    \t%s - %s - %s", FUNCS[k].name, format(v.count, f.count), format(v.size, f.size)))
      else
        print("\t  PARENT    \tnot registered")
      end
    end
    if ok then print("    \t /") end
  end

  print(string.format("FUNC\t%s - %d - %d bytes", f.name, f.count, f.size))

end
