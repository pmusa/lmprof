local lmprof = require"lmprof"

local LOOP = 100000000
local START = 0
local START_DIFF = LOOP/10
local STOP = 0
local STOP_DIFF = LOOP/20

for i=1,LOOP do
  if i == START then
    lmprof.start()
    STOP = START + STOP_DIFF
    START = START + START_DIFF
  elseif i == STOP then
    lmprof.stop()
  end
end
