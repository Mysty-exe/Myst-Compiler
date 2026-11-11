func swap(ref int a, ref int b):
    int temp = a
    a = b
    b = temp

func main() -> int:
    int x = 10
    int y = 99

    swap(x, y)

    print(x)
    print(y)
    return 0

main()
