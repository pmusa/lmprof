local lmprof = require"lmprof"
lmprof.start()

local function aux_cicle (ntab, tab, aux_tab)
  aux_tab[ntab] = 1
  for k,v in pairs(ntab) do
    if v == tab then
      return true
    end
    if type(v) == "table" and not aux_tab[v] then
      return aux_cicle(v, tab, aux_tab)
    end
  end
  return false
end

function cicle(tab)
  if tab then
    return aux_cicle(tab, tab, {})
  else
    return false
  end
end

a = {x = {}};  a.x[4] = a
cicle(a)
a = {x = {}};  a.x[4] = a.x
cicle(a)
a = {x = {}};  
a.x[4] = a.x
a.x[5] = x
b = {c = {a}}
a.x = b
cicle(a)

lmprof.stop()
