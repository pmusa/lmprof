local FUNCTIONS = dofile(... or "lmprof_output.txt")

local COUNT = FUNCTIONS.count
FUNCTIONS.count = nil
local SIZE = FUNCTIONS.size
FUNCTIONS.size= nil

local function format(fname, parent, count, size, tcount, tsize)
  local STR = '"%s" -> "%s" [label = "%s"]\n'
  local label = string.format("%d (%d%%) - %d (%d%%)", count, count/tcount, size, size/tsize) 
  return string.format(STR, parent, fname, label)
end

local nodes = "{ node [shape = circle]\n"
local links = ""
for fref, f in pairs(FUNCTIONS) do
  nodes = nodes .. '"' .. f.name .. fref .. '"\n'
  for pref, p in pairs(f.parents) do
    links = links .. format(f.name..fref, FUNCTIONS[pref].name, p.count, p.size, f.count, f.size)
  end
end

print(nodes)
print(links)
