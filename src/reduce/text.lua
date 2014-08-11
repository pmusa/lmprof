local REPORT = dofile(... or "lmprof_default_output.txt")

local ts = 0
local tc = 0
local order = {
  "mem_perc", "mem_cum", "mem_self", "calls", "mpc_cum", "mpc_self", "name"
}

local ROOT
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
    parents = {},
    children = {},
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

    -- always try to use the function name instead of (...tail calls...)
    local tail_call_pattern = "%(%.%.%.tail calls%.%.%.%)"
    if ft.name:match(tail_call_pattern) and not f.name:match(tail_call_pattern) then
      ft.name = f.name
    end
   
    ft.calls = ft.calls + f.calls 
    ft.mem_self = ft.mem_self + f.mem_self 
   
    if f.parent == "0" then
      ROOT = ft
    else
      local pt = ft.parents[f.parent] or {calls = 0, mem_self = 0} 
      if not ft.parents[f.parent] then 
        ft.parents[f.parent] = pt 
      end 
      pt.calls = pt.calls + f.calls 
      pt.mem_self = pt.mem_self + f.mem_self 
      PARENTS[f.parent] = true
    end
  end
  for fref,f in pairs(graph) do
    f.mem_perc = (f.mem_self/ts) * 100
    f.mpc_self = f.mem_self/f.calls
    f.mem_cum  = f.mem_cum + f.mem_self
-- [[
    for pref,p in pairs(f.parents) do
      if graph[pref] then
        if not graph[pref].children then
          graph[pref].children = {}
        end
        graph[pref].children[fref] = f
      end
    end
--]]
  end
  return graph
end

local update_tree_visited = {}
local function update_tree(p)
  if update_tree_visited[p] then
    return
  end
  update_tree_visited[p] = true
  for _,f in pairs(p.children) do
    update_tree(f)
    p.mem_cum = p.mem_cum + f.mem_cum
  end
  p.mpc_cum = p.mem_cum/p.calls
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
--for k,v in pairs(PARENTS) do if not graph[k] then print(k) end end
--local leafs = get_leafs(graph)
--update_graph(graph, leafs)
update_tree(ROOT)
local a = graph2array(graph)
local rc = dofile("ascii_table_print.lua")
rc.flat_print(a, 20)

--[[
for k,v in pairs(ROOT.children) do print(k,v) end
print(ROOT.children["30571184"])
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

local function update_graph (graph, leafs, func)
  repeat
    local current = {}
    for _,fref in ipairs(leafs) do
      local f = graph[fref]
      if f and not f.visited then
        f.visited = true
        for pref, p in pairs(f.parents) do
          if graph[pref] and not graph[pref].visited then
              graph[pref].mem_cum  = graph[pref].mem_cum + f.mem_self
              table.insert(current, pref)
          end
        end
      end
    end
    leafs = current
  until #current == 0
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

--]]
