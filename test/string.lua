local lmprof = require"lmprof"

-- pre-defined strings do not use malloc
local a = ""
local b = "t"
local c = "a"
local d = "tamanho8"
local e = "tamanhotamanhotamanhotamanhotamanhotamanhotamanhotamanhotamanho65"

lmprof.start("lmprof_string.lua")
assert((a..b))
assert((b..c))
assert((b..c..d))
assert((b..c..d..e))
