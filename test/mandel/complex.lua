-- complex.lua
-- complex arithmetic package for Lua
-- Luiz Henrique de Figueiredo (lhf@csg.uwaterloo.ca)
-- 24 Oct 95

Complex={type="package"}

function complex(x,y)
 return setmetatable({ re=x, im=y }, Complex.metatable)
end

function Complex.conj(x,y)
 return complex(x.re,-x.im)
end

function Complex.norm2(x)
 local n=Complex.mul(x,Complex.conj(x))
 return n.re
end

function Complex.abs(x)
 return sqrt(Complex.norm2(x))
end

function Complex.add(x,y)
 return complex(x.re+y.re,x.im+y.im)
end

function Complex.mul(x,y)
 return complex(x.re*y.re-x.im*y.im,x.re*y.im+x.im*y.re)
end

Complex.metatable={
	__add = Complex.add,
	__mul = Complex.mul,
}

function abs(x)
 return math.sqrt(Complex.norm2(x))
end
