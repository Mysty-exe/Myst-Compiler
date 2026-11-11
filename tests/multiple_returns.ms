func max(int a, int b) -> int:
    if a > b:
        return a
    return b


func clamp(int val, int minVal, int maxVal) -> int:
    if val < minVal:
        return minVal
    if val > maxVal:
        return maxVal
    return val

func main() -> int:
    print(max(15, 42))
    print(clamp(5, 10, 20))
    print(clamp(50, 10, 20))
    print(clamp(15, 10, 20))
    return 0

main()