local UNITS = {" ", "K", "M", "T", "P", "E", "Z", "Y"}

local function ntu (n)
  local c = 1
  while n > 999 do
    n = n / 1024
    c = c + 1
  end
  return string.format("%5.1f %sB", n, UNITS[c])
end

function flat_print(t, max)
  local ctla = 3  -- calls title line ajust
  local cs = 5  -- calls size
  for _,v in ipairs(t) do
    local l = string.len(v.calls)
    if l > cs then cs = l end
  end

  local sc = string.rep(" ", cs - ctla)
  local tl = [[
   %%   cumulative    self  %s      cumulative    self
  mem     mem        mem   %scalls   mem/call   mem/call  name]]
--[[
 __.__  ___._ kb   ___._ kb    ???  ___._ kb  ___._ kb  ???
--]]
  local fl = "%6.2f  %s   %s  %"..cs.."d  %s   %s  %s"

  print(string.format(tl, sc, sc))

  for i,v in ipairs(t) do
    if i == max+1 then break end
    local p = {}
    for i,k in ipairs(t.order) do
      if i == 2 or i == 3 or i == 5 or i == 6 then
        table.insert(p, ntu(v[k]))
      else
        table.insert(p, v[k])
      end
    end
    print(string.format(fl, table.unpack(p)))
  end
end

local function graph_print(funcs, ifuncs)
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
end

return {
  flat_print = flat_print,
}
