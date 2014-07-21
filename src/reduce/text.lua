local REPORT = dofile(... or "lmprof_default_output.txt")

local ts = 0
local tc = 0
local order = {
  "mem_perc", "mem_cum", "mem_self", "calls", "mpc_cum", "mpc_self", "name"
}

local PARENTS = {}

local function new_func(name)
  return {
    mem_perc = 0,
    mem_cum = 0,
    mem_self = 0, 
    calls = 0,
    mpc_cum = 0,
    mpc_self = 0,
    name = name,
    parents = {}
  }
end

local function report2graph(report)
  local graph = {}
  -- create bottom up graph 
  for _, f in pairs(report) do 
    tc = tc + f.calls 
    ts = ts + f.mem_self 
   
    local ft = graph[f.func] or new_func(f.name)
    if not graph[f.func] then 
      graph[f.func] = ft 
    end 
   
    ft.calls = ft.calls + f.calls 
    ft.mem_self = ft.mem_self + f.mem_self 
   
    local pt = ft.parents[f.parent] or {calls = 0, mem_self = 0} 
    if not ft.parents[f.parent] then 
      ft.parents[f.parent] = pt 
    end 
    pt.calls = pt.calls + f.calls 
    pt.mem_self = pt.mem_self + f.mem_self 
   
    PARENTS[f.parent] = true 
  end
  return graph
end

local function is_parent(fref)
  return PARENTS[fref]
end

-- get leafs
local function get_leafs (graph)
  local leafs = {}
  for k,_ in pairs(graph) do
    if not is_parent(k) then
      table.insert(leafs, k)
    end
  end
  return leafs
end

local function update_graph (graph, leafs)
  repeat
    local current = {}
    for _,fref in ipairs(leafs) do
      local f = graph[fref]
      if f then
        f.visited = true
        f.mem_perc = (f.mem_self/ts) * 100
        f.mpc_self = f.mem_self/f.calls
        f.mem_cum  = f.mem_cum + f.mem_self
        f.mpc_cum  = f.mem_cum/f.calls
        for pref, p in pairs(f.parents) do
          local parent = graph[pref]
          if parent and not parent.visited then
            parent.mem_cum = parent.mem_cum + f.mem_cum
            table.insert(current, pref)
          end
        end
      end
    end
    leafs = current
  until #current == 0
end 

local function graph2array (graph)
  local a = {order = order}
  for k, f in pairs(graph) do
    table.insert(a, f)
  end
  table.sort(a, function(a,b) return a.mem_perc > b.mem_perc end)
  return a
end

local graph = report2graph(REPORT)
local leafs = get_leafs(graph)
update_graph(graph, leafs)
local a = graph2array(graph)
local rc = dofile("ascii_table_print.lua")
rc.flat_print(a, 10)

--[[
for _, f in pairs(graph) do
  if f.size > 0 then
    table.insert(Igraph, f)
  end
end

table.sort(Igraph, function(a,b) return a.size > b.size end)

local X = 10
print(#Igraph .. " functions allocated data")
print(tc .. " calls allocated data")
print(string.format("%dMB were allocated", ts/1024/1024))
print("\n")
print("Top "..X.." allocations follows:")
print("------------------------------------------------------------")
--rp.flat_print(ifuncs)
--]]
