func doubleInPlace(ref int target):
    target = target * 2

func main() -> int:
    int num = 21
    doubleInPlace(num)
    print(num)
    return 0

main()
