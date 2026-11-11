import math_lib as Math

func main() -> int:
    Math::Vector v1 = Math::Vector(3, 4)
    Math::Vector v2 = Math::Vector(1, 2)
    Math::Vector res = Math::addVectors(v1, v2)
    
    print_int(res.x)
    print_int(res.y)
    return 0

main()