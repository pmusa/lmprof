local UNITS = {" ", "K", "M", "T", "P", "E", "Z", "Y"}

local flat_order = {
  "mem_perc", "mem_cum", "mem_self", "calls", "mpc_cum", "mpc_self", "name",
}
local call_graph_order = {
  "index", "mem_perc", "mem_self", "descendants", "calls", "name", "index",
}

-- number2unit
local function n2u (n)
  local c = 1
  while n > 999 do
    n = n / 1024
    c = c + 1
  end
  return string.format("%5.1f %sB", n, UNITS[c])
end

-- number to index
local function n2i (n)
  if not n then
    return string.rep(" ", 6)
  end
  local MAX_INDEX_LENGTH = 4
  local nsp = MAX_INDEX_LENGTH - string.len(n)
  local s = string.format("%s[%d]", string.rep(" ", nsp), n)
  return s
end

local X = {}

local function graph2ordered_array (func_table, sort_field)
  local a = {flat_order = flat_order}
  local i = 1
  for fref, f in pairs(func_table) do
    a[i] = f
    i = i + 1 
  end
  table.sort(a, function(a,b) return a[sort_field] > b[sort_field] end)
  for i, f in ipairs(a) do
    a[f.ref] = i
  end
  return a
end

function flat_print(graph, max)
  local t = graph2ordered_array(graph, "mem_perc")
  local ctla = 3  -- calls title line ajust
  local cs = 5  -- calls size
  for _,v in ipairs(t) do
    local l = string.len(v.calls)
    if l > cs then cs = l end
  end

  local sc = string.rep(" ", cs - ctla)
  -- title line flat pattern
  local tlfp = [[
   %%   cumulative   self  %s        cumulative    self
  mem     mem        mem   %scalls   mem/call   mem/call  name]]
--[[
 __.__  ___._ kb   ___._ kb    ???  ___._ kb  ___._ kb  ???
  function line flat pattern
--]]
  local flfp = "%6.2f  %s   %s  %"..cs.."d   %s   %s  %s"

  print(string.format(tlfp, sc, sc))

  for i,v in ipairs(t) do
    if i == max+1 then break end
    local p = {}
    for i,k in ipairs(t.flat_order) do
      if i == 2 or i == 3 or i == 5 or i == 6 then
        table.insert(p, n2u(v[k]))
      else
        table.insert(p, v[k])
      end
    end
    print(string.format(flfp, table.unpack(p)))
  end
end

local function call_graph_print(func_table)
  local a = graph2ordered_array(func_table, "mem_perc")

  local ctla = 5  -- calls title line ajust
  local cs = 5    -- calls size
  local ntla = 5  -- name title line ajust
  local ns = 8    -- name size
  for _,v in ipairs(a) do
    local l
    l = string.len(v.calls)
    if l > cs then cs = l end
    l = string.len(v.name)
    if l > ns then ns = l end
  end

--[[
                                          called/total       parents
 index    %mem     self    descendants     called+self      name           index
                                          called/total       children
 [9999]  100.00  999.9 MB   999.99 MB   999 *10^3/999*10^3  asdasa...asdd  [9999]
--]] 
-- TitleLineCallGraphPattern & FunctionLineCallGraphPattern
  local tlcgp = {
"                                      %scalled/total%s    parents",
" index    %%mem     self   cumulative  %scalled+self %s  name      %sindex",
"                                      %scalled/total%s    children",
  }

  local sc = string.rep(" ", cs - ctla)
  local sn = string.rep(" ", ns - ntla)
  print(string.format(tlcgp[1], sc, sc))
  print(string.format(tlcgp[2], sc, sc, sn))
  print(string.format(tlcgp[3], sc, sc))
  -- minus 6 is due to %s
  local separator_len = string.len(tlcgp[2])+string.len(sc)+string.len(sn)-6
  local separator = string.rep("-", separator_len)
  print(separator)

  local cp = "%" .. cs .. "d/%d"
  local flcgpp = string.rep(" ", 16) .. "%s   %s    %" .. cs .. "d/%d  %s  %s%s  %s" 
  local flcgfp = "%s  %6.2f  %s   %s    %" .. cs .. "d+%d%s  %s%s    %s"

  for i,f in ipairs(a) do
    for pref,p in pairs(f.parents) do
      local adjc = string.rep(" ", cs - string.len(f.calls))
      local adjn = string.rep(" ", ns - string.len(func_table[pref].name))
      print(string.format(flcgpp, n2u(p.mem_self), n2u(p.mem_cum), p.calls, f.calls, adjc, func_table[pref].name, adjn, n2i(a[pref])))
    end

    local adjc = string.rep(" ", cs - string.len(f.calls_rec or 0))
    local adjn = string.rep(" ", ns - string.len(f.name))
    print(string.format(flcgfp, n2i(i), f.mem_perc, n2u(f.mem_self), n2u(f.mem_cum), f.calls, f.calls_rec or 0, adjc, f.name, adjn, n2i(i)))

    for cref, c in pairs(f.children) do
      local adjc = string.rep(" ", cs - string.len(func_table[cref].calls))
      local adjn = string.rep(" ", ns - string.len(func_table[cref].name))
      print(string.format(flcgpp, n2u(c.mem_self), n2u(c.mem_cum), c.calls, func_table[cref].calls, adjc, func_table[cref].name, adjn, n2i(a[cref])))
    end
    print(separator)
  end
end

return {
  flat_print = flat_print,
  call_graph_print = call_graph_print,
}
