// --- 1. Implicit Casting Tests ---

// Function taking a decimal parameter (int argument should cast implicitly)
func process_decimal(decimal d) -> decimal:
    return d + 2.5

// Function returning decimal, but returning an int (implicit return cast)
func get_as_decimal() -> decimal:
    int raw_val = 10
    return raw_val

// Variable declaration implicit casting (assigning int literal to decimal)
decimal inferred_dec = 5
print(inferred_dec)

// Function parameter implicit casting (passing int literal to decimal parameter)
decimal param_cast_res = process_decimal(20)
print(param_cast_res)

// Function return implicit casting
decimal return_cast_res = get_as_decimal()
print(return_cast_res)


// --- 2. Explicit Casting Tests ---

decimal original_dec = 42.75
int converted_int = toInt(original_dec)
decimal back_to_dec = toDecimal(converted_int)

bool truthy_val = toBool(1)
bool falsy_val = toBool(0)

print(converted_int)
print(back_to_dec)
print(truthy_val)
print(falsy_val)