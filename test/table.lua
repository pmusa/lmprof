-- Author: Pablo Musa
-- Creation Date: jun 20 2011
-- Last Modification: aug 09 2011

local lmprof = require"lmprof"
lmprof.start()
local t = {}
local t = {10}
local t = {10, 20, 30}
local t = {}
for i=10,200,10 do
  table.insert(t, i)
end
local t = {}
for i=10,320,10 do
  table.insert(t, i)
end
local t = {}
for i=10,330,10 do
  table.insert(t, i)
end
