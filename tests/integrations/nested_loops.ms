func main() -> int:
    int outer = 1
    int total = 0

    while outer <= 3:
        int inner = 1
        while inner <= 3:
            total = total + (outer * inner)
            inner = inner + 1
        outer = outer + 1

    print(total)
    return 0

main()
