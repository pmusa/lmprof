
local function get_name(t, f)
  return t[f] and t[f].name
end

function reduce(mem_report)
  for fref, f in pairs(mem_report) do
    for pref, p in pairs(f.parents) do
      pname = get_name(mem_report, pref)
      if name then
      end
    end
  end
end
