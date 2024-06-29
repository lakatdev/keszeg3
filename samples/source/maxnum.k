(rt MAIN
	print const n:
	input $ length

	i = 0
	while i < length
		disp = i
		disp +=
		print $ disp
		print const .:
		input $ tmp
		numbers : i <= tmp
		i +=
	end

	max <= numbers : 0
	i = 1
	while i < length
		tmp <= numbers : i
		if tmp > max
			max = tmp
		end
		i +=
	end

	print const max:
	print $ max
	print const \n
return)
