local exts = {
  "mov",
  "mkv",
  "h264",
  "mpeg",
  "mpeg4",
}

local r = {}
for _,ext in ipairs(exts) do
  r[string.lower(ext)] = true
end

return r

