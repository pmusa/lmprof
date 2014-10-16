local lt2oa = require("lmprof.lt2oa")


local UNITS = {"K", "M", "G", "T", "P", "E", "Z", "Y"}
local NUMBER_FORMAT = {"%5.1f", "%5.0f", "%5.2f"}

local flat_order = {
  "mem_perc", "mem_self", "mem_cum", "calls", "mpc_self", "mpc_cum", "name", 
}

-- max number length to format function
local function mnl2ff (len)
  if len <= 4 then
    return function (n)
      return string.format("%6d B", n)
    end
  end
  local nf = NUMBER_FORMAT[(len%3)+1] .. " %sB" -- lua table starts at 1
  local x = math.floor((len - 2)/3)
  local unit = UNITS[x]
  return function (n)
    return string.format(nf, n/(1024^x), unit)
  end
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

-- max len and memory size format table
local function mlmsft(ftarray, max, func_table)
  -- calculate max len for each field
  local maxlen = {
    calls = 5, mem_self = 1, mem_cum = 1, mpc_self = 1, mpc_cum = 1, name = 4,
  }
  for i,ft in ipairs(ftarray) do
    if max and i == max+1 then break end
    for k,mv in pairs(maxlen) do
      -- if number, use only integer part
      local l = type(ft[k]) == "number" and math.floor(ft[k]) or ft[k]
      l = string.len(l)
      maxlen[k] = l > mv and l or maxlen[k]
    end

    -- calculate parents field also
    if func_table then
      for pref, pt in pairs(ft.parents) do
        for k,mv in pairs(maxlen) do
          if not pt[k] then
            pt = func_table[pref]
          end
          local l = type(pt[k]) == "number" and math.floor(pt[k]) or pt[k]
          l = string.len(l)
          maxlen[k] = l > mv and l or maxlen[k]
        end
      end
    end
  end

  -- set memory size format function for each field
  local msft = {}  -- memory size format table
  for k,v in pairs(maxlen) do
    msft[k] = mnl2ff(v)
  end

  return maxlen, msft
end

function flat_print(func_table, max)
  local ftarray = lt2oa(func_table)

  -- get maxlen for numeric fields and format for memory size fields
  local maxlen, msft = mlmsft(ftarray, max)

  local ctla = 3  -- calls title line ajust
  local sc = string.rep(" ", maxlen.calls - ctla)
  print(string.format([[
   %%      self    cumulative %s         self    cumulative
  mem     mem        mem    %scalls   mem/call   mem/call   name]], sc, sc))

  -- function line flat pattern
  local flfp = "%6.2f  %s   %s   %"..maxlen.calls.."d  %s    %s   %s"
  for i,v in ipairs(ftarray) do
    if max and i == max+1 then break end
    local p = {}
    for i,k in ipairs(flat_order) do
      if k:match("cum") or k:match("self") then
        table.insert(p, msft[k](v[k]))
      else
        table.insert(p, v[k])
      end
    end
    print(string.format(flfp, table.unpack(p)))
  end
end

local function call_graph_print(func_table, max)
  local ftarray = lt2oa(func_table)

  -- get maxlen for numeric fields and format for memory size fields
  local maxlen, msft = mlmsft(ftarray, max, func_table)

  -- [[ print header ]] --
  local header = {
"                                      %scalled/total%s  parents",
" index    %%mem    self    cumulative  %scalled (rec)%s  name  %sindex",
"                                      %scalled/total%s  children",
  }

  local ctla = 5  -- calls title line ajust
  local ntla = 4  -- name title line adjust
  local sc = string.rep(" ", maxlen.calls - ctla)
  local sn = string.rep(" ", maxlen.name - ntla)
  print(string.format(header[1], sc, sc))
  print(string.format(header[2], sc, sc, sn))
  print(string.format(header[3], sc, sc))

  -- [[ set separator, the minus 6 is due to %s ]] --
  local separator_len = string.len(header[2])+string.len(sc)+string.len(sn)-7
  local separator = string.rep("=", separator_len)

  print(separator)

  
  local flcgpp = string.rep(" ", 16) .. "%s   %s    %" .. maxlen.calls .. "d/%d%s  %s%s %s" 
  local flcgfp = "%s  %6.2f  %s   %s    %" .. maxlen.calls .. "d (%d)%s%s%s %s"

  local function cglprint(ms, mc, c, oc, n, i, mp)
    local adjc = string.rep(" ", maxlen.calls - string.len(oc))
    local adjn = string.rep(" ", maxlen.name - string.len(n))
    if not mp then  -- parent or child
      print(string.format(flcgpp, ms, mc, c, oc, adjc, n, adjn, i))
    else  -- the current function
      local l
      if oc ~= 0 then  -- there are recursive calls
        l = string.format(flcgfp, i, mp, ms, mc, c, oc, adjc, n, adjn, i)
      else  -- no recursive calls
        local tmp = string.gsub(flcgfp, "%(%%d%)", "%%s")
        adjc = string.rep(" ", maxlen.calls + 2)  -- plus 2 for ()
        oc = ""
        l = string.format(tmp, i, mp, ms, mc, c, oc, adjc, n, adjn, i)
      end
      local _, i = string.find(l, "%s*")
      local l = string.rep(" ", i) .. string.gsub(string.sub(l, i+1), " ", "-")
      print(l)
    end
  end

  for i,f in ipairs(ftarray) do
    if max and i == max+1 then break end

    local pa = lt2oa(f.parents)
    for _,p in ipairs(pa) do
      cglprint(msft.mem_self(p.mem_self), msft.mem_cum(p.mem_cum), p.calls,
               f.calls, func_table[p.parent].name, n2i(ftarray[p.parent]))
    end

    cglprint(msft.mem_self(f.mem_self), msft.mem_cum(f.mem_cum), f.calls,
             f.calls_rec or 0, f.name, n2i(i), f.mem_perc)

    local ca = lt2oa(f.children)
    for _, c in ipairs(ca) do
      cglprint(msft.mem_self(c.mem_self), msft.mem_cum(c.mem_cum), c.calls,
               func_table[c.ref].calls, func_table[c.ref].name, n2i(ftarray[c.ref]))
    end
    print(separator)
  end
end

return {
  flat_print = flat_print,
  call_graph_print = call_graph_print,
}
