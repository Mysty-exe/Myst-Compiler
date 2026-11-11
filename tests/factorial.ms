func computeFactorial(int n) -> int:
    int result = 1
    int i = 1
    
    while i <= n:
        result = result * i
        i = i + 1
        
    return result

int val = 5
int fact = computeFactorial(val)
print(fact)
