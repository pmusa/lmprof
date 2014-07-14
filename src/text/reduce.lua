local FUNCTIONS = {}
local CHILDREN = dofile(... or "lmprof_output.txt")
local PARENTS = {}

local COUNT = CHILDREN.count
CHILDREN.count = nil
local SIZE = CHILDREN.size
CHILDREN.size= nil

for fref, f in pairs(CHILDREN) do
  FUNCTIONS[fref] = { name = f.name, count = f.count, size = f.size }
  for pref, p in pairs(f.parents) do
    if not PARENTS[pref] then
      PARENTS[pref] = {
        name = CHILDREN[pref].name,
        count = 0,
        size = 0,
        children = {},
      }
    end
    PARENTS[pref].count = PARENTS[pref].count + p.count
    PARENTS[pref].size = PARENTS[pref].size + p.size
    PARENTS[pref].children[fref] = {name = f.name, count = p.count, size = p.size}
  end
end

local function format(parc, total)
  return string.format("%d (%d%%)", parc, parc/total*100)
end

for key, f in pairs(FUNCTIONS) do
         ----------------------
  print("-----------------------Fname-------------count------------size----------")
  if CHILDREN[key] then
    local ok = false
    for k,v in pairs(CHILDREN[key].parents) do
      ok = true
      print("\t  PARENT    ", FUNCTIONS[k].name, "", format(v.count, f.count), "", format(v.size, f.size))
    end
    if ok then print("    \t /") end
  end

  print("FUNC", f.name, "\t\t", f.count, "", f.size.." bytes")
  print("FUNC", string.format("%.7s", f.name), "\t\t", f.count, "", f.size.." bytes")

  if PARENTS[key] then
    print("    \t \\")
    for k,v in pairs(PARENTS[key].children) do
      print("\t  CHILD     ", v.name, "", v.count, "", v.size)
    end
  end
end
print("------------------------------------------------------------------------")

