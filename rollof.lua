local lmprof = require"lmprof"

lmprof.start()

VIDEO_EXTENSIONS = dofile("extensions.lua")
local FOLDER = arg[1] or "/home/pmusa/Videos"

local function isvideo(file_name)
  local ext = file_name:match(".+%.(%w*)$")
  ext = ext and string.lower(ext) or nil
  return VIDEO_EXTENSIONS[ext]
end

local function avtime2s(str)
  local h,m,s,ms = string.match(str, "^(%d%d%d?):(%d%d):(%d%d)%.%d*$")
  return h * 3600 + m * 60 + s
end

local function s2avtime(time)
  local h = math.floor(time / 3600)
  local m = math.floor( (time - (h * 3600)) / 60)
  local s = time - (h * 3600) - (m * 60)
  return string.format("%d:%d:%d", h, m, s)
end

local total_time, i = 0, 0;
local file_list = io.popen("find " .. FOLDER)
for file in file_list:lines() do
  if isvideo(file) then
    local cmd = string.format('avconv -i "%s" 2>&1', file)
print(cmd)
    video_stats = io.popen(cmd)
    for stats in video_stats:lines() do
      local time = stats:match("Duration: (%d%d:%d%d:%d%d%.%d%d), .*")
      if time then
        print(file, time)
        total_time = total_time + avtime2s(time)
        i = i + 1
      end
    end
  end
end

print(string.format("Arquivos: %d - Tempo Total: %s", i, s2avtime(total_time)))

lmprof.stop()
