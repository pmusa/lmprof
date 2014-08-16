local lmprof = require"lmprof"
lmprof.start()

dofile("resources/complex.lua")

xmin=-2		xmax=2		ymin=-2		ymax=2
--sky
--N=256
N=16

function print(...)
 local arg = table.pack(...)
 for i=1,arg.n do
  io.write(arg[i]," ")
 end
 io.write("\n")
end

function level(x,y)
 local c=complex(x,y)
 local l=0
 local z=c
 repeat
  z=z*z+c
  l=l+1
 until abs(z)>2 or l>255
 return l-1
end

dx=(xmax-xmin)/N
dy=(ymax-ymin)/N

print("P2")
print("# mandelbrot set",xmin,xmax,ymin,ymax,N)
print(N,N,255)
for i=1,N do
 local x=xmin+(i-1)*dx
 for j=1,N do
  local y=ymin+(j-1)*dy
  print(level(x,y))
 end
end

lmprof.stop()
