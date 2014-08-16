local fn1 = arg[1]
local fn2 = arg[2]

if not fn2 then
  print("usage: " .. arg[0] .. " <file1> <file2>")
  os.exit(1)
end

local reduce = require"lmprof.reduce"
local lt2oa = require"lmprof.lt2oa"


ft1 = lt2oa(reduce(dofile(fn1)))
ft2 = lt2oa(reduce(dofile(fn2)))

local error_rate = 20/100
local diff_item = 0
for k,func in ipairs(ft1) do
  for field, v1 in pairs(func) do
    if type(v1) ~= "table" and field ~= "ref" then
      local v2 = ft2[k][field]
      if tonumber(v1) then
        local b,s
        if v1 > v2 then b,s = v1,v2
        else b,s = v2,v1 end
        if b + (b/error_rate) < s or b - (b/error_rate) > s then
          diff_item = diff_item + 1
        end
      elseif v1 ~= v2 then
        diff_item = diff_item + 1
      end
    end
  end
end

os.exit(diff_item)
