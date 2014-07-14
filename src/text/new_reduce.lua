local FUNCTIONS = dofile(... or "lmprof_output.txt")

local COUNT = FUNCTIONS.count
FUNCTIONS.count = nil
local SIZE = FUNCTIONS.size
FUNCTIONS.size= nil

local ORDERED_FUNCTIONS = {}
for fref, f in pairs(FUNCTIONS) do
  for pref, p in pairs(f.parents) do
    if not FUNCTIONS[pref] then
      PARENTS[pref] = {
        name = CHILDREN[pref].name,
        count = 0,
        size = 0,
        children = {},
      }
    elseif not FUNCTIONS[pref].children then
      FUNCTIONS[pref].children = {}
    end
    FUNCTIONS[pref].children[fref] = {name = f.name, count = p.count, size = p.size}
  end
  table.insert(ORDERED_FUNCTIONS, f)
end

local function format(parc, total)
  return string.format("%d (%d%%)", parc, parc/total*100)
end


for _, f in pairs(FUNCTIONS) do
         ----------------------
  print("-----------------------Fname-------------count------------size----------")
  if f.parents then
    local ok = false
    for k,v in pairs(f.parents) do
      ok = true
      print("\t  PARENT    ", FUNCTIONS[k].name, "", format(v.count, f.count), "", format(v.size, f.size))
    end
    if ok then print("    \t /") end
  end

  print("FUNC", f.name, "\t\t", f.count, "", f.size.." bytes")

  if f.children then
    print("    \t \\")
    for k,v in pairs(f.children) do
      print("\t  CHILD     ", v.name, "", v.count, "", v.size)
    end
  end
end
print("------------------------------------------------------------------------")

table.sort(ORDERED_FUNCTIONS, function(a,b) return a.size > b.size end)

for i=1,10 do
  local f = ORDERED_FUNCTIONS[i]
  print(f.name, f.size)
end
print(#ORDERED_FUNCTIONS)
