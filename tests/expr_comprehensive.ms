func helper(int x) -> int:
    return x * 2

func main() -> int:
    // 1. Basic Literals and Identifiers
    int a = 10
    int b = 5
    print(a)
    print(b)

    // 2. Unary with Unary
    int u1 = -(-15)
    int u2 = -(-(-8))
    print(u1)
    print(u2)

    // 3. Binary with Unary & Unary with Binary
    int bu1 = a + -b
    int ub1 = -(a + b)
    int bu2 = -a * -2
    print(bu1)
    print(ub1)
    print(bu2)

    // 4. Binary with Binary (Precedence and Parentheses)
    int bb1 = 2 + 3 * 4
    int bb2 = (2 + 3) * 4
    int bb3 = 20 / 4 - 2 * 3
    print(bb1)
    print(bb2)
    print(bb3)

    // 5. Call Expressions combined with Unary and Binary
    int c1 = helper(4)
    int c2 = helper(3) + 5
    int c3 = -helper(5) + helper(2) * 3
    print(c1)
    print(c2)
    print(c3)

    // 6. Complex Nested Expression (Identifiers, Calls, Unary, and Binary)
    int complex_val = -((a + b) * helper(2) - -5) / 2
    print(complex_val)

    return 0

main()
