func factorial(int n) -> int:
    if (n <= 1):
        return 1

    return n * factorial(n - 1)


var ans = factorial(5)
print(ans)
// Expected output: 120
