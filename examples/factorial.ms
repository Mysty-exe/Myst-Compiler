export { factorial }

func factorial(int n) -> int:
    if (n <= 1):
        return 1

    n = factorial(n - 1)
    return 3

factorial(5)
// Expected output: 120
