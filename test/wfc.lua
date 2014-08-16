local lmprof = require"lmprof"
lmprof.start(...)

function allwords ()
	local line = io.read()  -- current line
	local pos = 1           -- current position in the line
	return function ()      -- iterator function
		while line do         -- repeat while there are lines
			local s, e = string.find(line, "%w+", pos)
			if s then           -- found a word?
				pos = e + 1       -- next position is after this word
				return string.sub(line, s, e)     -- return the word
			else
				line = io.read()  -- word not found; try next line
				pos = 1           -- restart from first position
			end
		end
		return nil            -- no more lines: end of traversal
	end
end

function word_count(f, max)
	local tab,k,result,word,n = {},0,{}
	io.input(f)
	for word in allwords() do
		if tab[word] then
			tab[word].qtd = tab[word].qtd + 1
		else
			tab[word] = {}
			tab[word].qtd = 1
			tab[word].palavra = word
		end
	end
	for k,n in pairs(tab) do table.insert(result,n) end
	table.sort(result, function (a, b) 
		return a.qtd > b.qtd
	end )
  local b=0;
  print(#result)
	for k,v in ipairs(result) do 
		if b ==max then return end
    b = b+1
		print(v.palavra,v.qtd)
  end
end

local old_in = io.input()
word_count("resources/wfc.in", 10)
io.input(old_in)
lmprof.stop()
