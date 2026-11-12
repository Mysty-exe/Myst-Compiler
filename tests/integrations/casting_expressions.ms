func evaluate_math(decimal val) -> decimal:
    // Unary minus combined with a binary expression and implicit parameter casting
    return -val + 10.5

// 1. Implicit casting with unary prefix and binary operations
decimal neg_decimal = -15 + 3.5 // int -15 implicitly cast to decimal, unary minus applied
print(neg_decimal)

// 2. Explicit casting combined with unary minus and binary arithmetic
int mixed_val = -toInt(48.9) / 2 + -10
print(mixed_val)

// 3. Unary boolean negation combined with explicit casting
bool check_val = toBool(32)
print(check_val)

// 4. Function call passing a unary/binary expression with implicit parameter casting
decimal fn_result = evaluate_math(-20.0 * 2)
print(fn_result)

// 5. Complex chaining: Unary prefix, binary operations, and explicit casting together
decimal complex_res = toDecimal(-toInt(15.7) + 5) * -2.0
print(complex_res)
