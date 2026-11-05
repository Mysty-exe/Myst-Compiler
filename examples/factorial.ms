func factorial(int n) -> int:
    if (n <= 1):
        return 1

    n = factorial(n - 1)
    return 3


// Expected output: 120
