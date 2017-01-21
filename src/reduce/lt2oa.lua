-- lt2oa: lmprof table to ordered array

local function lmprof_sort(sort_field)
  return function(a,b)
  if sort_field and a[sort_field] ~= b[sort_field] then
    return a[sort_field] > b[sort_field]
  end
  
  if a.mem_self == b.mem_self then
    if a.mem_cum == b.mem_cum then
      if a.calls == b.calls and a.name and b.name then
        return a.name > b.name
      end
      return a.calls > b.calls
    end
    return a.mem_cum > b.mem_cum
  end
  return a.mem_self > b.mem_self
  end
end

local function lmprof_table2ordered_array (func_table, sort_field)
  local a = {}
  local i = 1
  for _, f in pairs(func_table) do
    a[i] = f
    i = i + 1
  end
  table.sort(a, lmprof_sort(sort_field))
  for k, f in ipairs(a) do
    a[f.ref] = k
  end
  return a
end

return lmprof_table2ordered_array
