import modules.utility_lib as Util

func main() -> int:
    int num = 7
    int sq = Util::computeSquare(num)
    int check = Util::isEven(sq)
    
    print(Util::SQUARE_FACTOR)
    print(sq)
    print(check)
    return 0

main()
