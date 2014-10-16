-- load luaproc
luaproc = require "luaproc"
require"lmprof".start("lmprof_main.lua")

local t = {}
for i=1,100 do table.insert(t, i) end

-- create an additional worker
luaproc.setnumworkers( 2 )

-- create a new lua process
luaproc.newproc( [[
  -- load lmprof
  table = require("table")
  local lmprof = require("lmprof")
  lmprof.start("lmprof_channel.lua")

  -- dummy allocation
  local t = {}
  for i=1,100 do table.insert(t, i) end

  -- create a communication channel
  luaproc.newchannel( "achannel" )
  -- create a sender lua process
  luaproc.newproc( [=[
    -- load lmprof
    table = require("table")
    local lmprof = require("lmprof")
    lmprof.start("lmprof_sender.lua")

    -- dummy allocation
    local t = {}
    for i=1,100 do table.insert(t, i) end

    -- send a message
    luaproc.send( "achannel", "hello world from luaproc" )
  ]=] )
  -- create a receiver lua process
  luaproc.newproc( [=[
    -- load lmprof
    table = require("table")
    local lmprof = require("lmprof")
    lmprof.start("lmprof_receiver.lua")

    -- dummy allocation
    local t = {}
    for i=1,100 do table.insert(t, i) end

    -- receive and print a message
    print( luaproc.receive( "achannel" ))
  ]=] )
]] )
