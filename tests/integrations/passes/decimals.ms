func calculateCircleArea(decimal radius) -> decimal:
    decimal pi = 3.14159265
    return pi * radius * radius

func main() -> int:
    decimal r = 2.5
    decimal area = calculateCircleArea(r)
    print(area)
    return 0

main()
