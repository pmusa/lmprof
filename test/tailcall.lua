local lmprof = require"lmprof"
lmprof.start()

local function d(t)
  if #t == 0 then return end
  local a = {}
  return d(table.remove(t) and t)
end

local function c()
  local t2 = {}
  for i=1,100 do t2[i] = i*i end
  return d(t2)
end


function b(t)
  local t2 = {}
  if true then
    for i=1,#t do table.insert(t2, t[i]) end
  end
  return c()
end

function a()
  local t = {1, 2, 3, 4, 5, 6, 7}
  b(t)
end

a()

lmprof.stop()
