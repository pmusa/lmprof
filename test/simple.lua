local lmprof = require"lmprof"

lmprof.start()

local g = function(f) f() end
local f1 = function() return 10 end
local f2 = function()
  local s = "test"
  local t = {}
  for i=1,1000 do
    table.insert(t, s..1)
  end 
end

local f3 = function()
  local s = "novo"
  local t = {}
  for i=1,1000 do
    table.insert(t, s..1)
  end 
end

g(f1)
g(f2)
g(f2)

lmprof.stop()

g(f1)
g(f3)

