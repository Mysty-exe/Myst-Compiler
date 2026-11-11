func testShadowing(int x) -> int:
    int result = x
    if x > 5:
        int x = 100
        result = result + x

    return result + x

func main() -> int:
    print(testShadowing(10))
    return 0

main()
