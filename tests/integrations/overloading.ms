// Test function overload resolution with exact matching vs implicit casting

func getNum(int x) -> int:
    print(1)
    return 1

func getNum(decimal x) -> int:
    print(2)
    return 2

// Test 1: Exact match for decimal should call getNum(decimal) -> prints 2
decimal d = 42.5
getNum(d)

// Test 2: Exact match for int should call getNum(int) -> prints 1
int i = 10
getNum(i)

// Test 3: Literal decimal should match getNum(decimal) -> prints 2
getNum(3.14)

// Test 4: Literal int should match getNum(int) -> prints 1
getNum(100)