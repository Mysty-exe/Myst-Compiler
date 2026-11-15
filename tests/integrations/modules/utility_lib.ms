export { SQUARE_FACTOR, computeSquare, isEven }

int SQUARE_FACTOR = 2

func computeSquare(int val) -> int:
    return val * val

func isEven(int val) -> int:
    if val - ((val / 2) * 2) == 0:
        return 1
    return 0
