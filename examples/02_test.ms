func process(int n) -> int:
	var count = 0
	while n > 0:
		if n == 5:
			count = count + 10
		else:
			count = count + 1
		n = n - 1
	return count