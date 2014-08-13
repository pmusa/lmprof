local REPORT = dofile(... or "lmprof_default_output.txt")
local nprint = require("ascii_table_print")

local ts = 0
local tc = 0

local ROOT
local function new_func(name, ref)
  return {
    ref = ref,
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

local function report2table_and_graph(report)
  local func_table = {}
  local func_graph

  -- iterate report table and create graph with basic info and parent info
  -- (call, mem)
  for _, f in pairs(report) do 
    tc = tc + f.calls 
    ts = ts + f.mem_self 
   
    local ft = func_table[f.func] or new_func(f.name, f.func)
    if not func_table[f.func] then 
      func_table[f.func] = ft 
    end 

    -- always try to use the function name instead of (...tail calls...)
    local tail_call_pattern = "%(%.%.%.tail calls%.%.%.%)"
    if ft.name:match(tail_call_pattern) and not f.name:match(tail_call_pattern) then
      ft.name = f.name
    end
   
    ft.calls = ft.calls + f.calls
    ft.mem_self = ft.mem_self + f.mem_self
    ft.mem_cum  = ft.mem_cum + f.mem_cum

    if f.parent == "0" then
      func_graph = ft
    elseif f.parent == f.func then
      ft.calls_rec = f.calls
      ft.mem_rec = f.mem_self
    else
      local pt = ft.parents[f.parent] or {calls = 0, mem_self = 0, mem_cum = 0}
      if not ft.parents[f.parent] then
        ft.parents[f.parent] = pt
      end 
      pt.calls = pt.calls + f.calls
      pt.mem_self = pt.mem_self + f.mem_self
      pt.mem_cum = pt.mem_cum + f.mem_cum
    end
  end

  -- iterate function nodes, calculate node accumulated values and set children
  for fref,f in pairs(func_table) do
    f.mem_perc = (f.mem_self/ts) * 100
    f.mpc_self = f.mem_self/f.calls
    f.mpc_cum  = f.mem_cum/f.calls
    for pref,pt in pairs(f.parents) do
      local p = func_table[pref]
      if p then
        if not p.children then
          p.children = {}
        end
        p.children[fref] = pt
      end
    end
  end
  
  return func_table, func_graph
end

local func_table, graph = report2table_and_graph(REPORT)
--nprint.flat_print(func_table, 20)
nprint.call_graph_print(func_table)

