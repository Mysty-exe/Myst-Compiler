#include <catch2/catch_test_macros.hpp>
#include "compiler/lexer.h"

Lexer lexer = Lexer();

TEST_CASE("Lexer tokenizes basic primitives and declarations", "[lexer]")
{
    std::string source = "int x = 42";
    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 1);
    REQUIRE(tokens[0].size() >= 4);
    REQUIRE(tokens[0][0].getToken() == "INTTYPE");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "x");
    REQUIRE(tokens[0][2].getToken() == "ASSIGN");
    REQUIRE(tokens[0][3].getToken() == "INT");
    REQUIRE(tokens[0][3].getValue() == "42");
}

TEST_CASE("Lexer handles decimal types and floating-point literals", "[lexer]")
{
    std::string source = "decimal y = 342.3";
    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 1);
    REQUIRE(tokens[0].size() >= 4);
    REQUIRE(tokens[0][0].getToken() == "DECIMALTYPE");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "y");
    REQUIRE(tokens[0][2].getToken() == "ASSIGN");
    REQUIRE(tokens[0][3].getToken() == "DECIMAL");
    REQUIRE(tokens[0][3].getValue() == "342.3");
}

TEST_CASE("Lexer handles character declarations and literals", "[lexer]")
{
    std::string source = "char c = 'c'";
    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 1);
    REQUIRE(tokens[0].size() >= 4);
    REQUIRE(tokens[0][0].getToken() == "CHARTYPE");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "c");
    REQUIRE(tokens[0][2].getToken() == "ASSIGN");
    REQUIRE(tokens[0][3].getToken() == "CHAR");
    REQUIRE(tokens[0][3].getValue() == "'c'");
}

TEST_CASE("Lexer distinguishes binary and unary operators in expressions", "[lexer]")
{
    std::string source = "int result = a + -b";
    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 1);
    REQUIRE(tokens[0].size() >= 7);
    REQUIRE(tokens[0][0].getToken() == "INTTYPE");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "result");
    REQUIRE(tokens[0][2].getToken() == "ASSIGN");
    REQUIRE(tokens[0][3].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][3].getValue() == "a");
    REQUIRE(tokens[0][4].getToken() == "PLUS");
    REQUIRE(tokens[0][5].getToken() == "MINUS"); // Unary or binary minus representation
    REQUIRE(tokens[0][6].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][6].getValue() == "b");
}

TEST_CASE("Lexer correctly handles multi-line blocks, functions, and indentation", "[lexer]")
{
    std::string source =
        "func add(int a, int b) -> int:\n"
        "    return a + b";

    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    // Line 0: func add(int a, int b) -> int:
    REQUIRE(tokens.size() >= 2);
    REQUIRE(tokens[0][0].getToken() == "FUNC");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "add");
    REQUIRE(tokens[0][2].getToken() == "LPAREN");
    REQUIRE(tokens[0][3].getToken() == "INTTYPE");
    REQUIRE(tokens[0][4].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][4].getValue() == "a");
    REQUIRE(tokens[0][5].getToken() == "COMMA");
    REQUIRE(tokens[0][6].getToken() == "INTTYPE");
    REQUIRE(tokens[0][7].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][7].getValue() == "b");
    REQUIRE(tokens[0][8].getToken() == "RPAREN");
    REQUIRE(tokens[0][9].getToken() == "ARROW");
    REQUIRE(tokens[0][10].getToken() == "INTTYPE");
    REQUIRE(tokens[0][11].getToken() == "COLON");

    // Line 1: return a + b (checking indentation and body tokens)
    REQUIRE(tokens[1][0].getToken() == "INDENT");
    REQUIRE(tokens[1][1].getToken() == "RETURN");
    REQUIRE(tokens[1][2].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][2].getValue() == "a");
    REQUIRE(tokens[1][3].getToken() == "PLUS");
    REQUIRE(tokens[1][4].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][4].getValue() == "b");
}

// TEST_CASE("Lexer tokenizes containers and custom types", "[lexer]")
// {
//     std::string source =
//         "container Person:\n"
//         "    int id\n"
//         "    int age";

//     lexer.tokenizeFile(source);
//     auto tokens = lexer.getTokens();

//     // Line 0: container Person:
//     REQUIRE(tokens.size() >= 3);
//     REQUIRE(tokens[0][0].getToken() == "CONTAINER");
//     REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
//     REQUIRE(tokens[0][1].getValue() == "Person");
//     REQUIRE(tokens[0][2].getToken() == "COLON");

//     // Line 1: int id
//     REQUIRE(tokens[1][0].getToken() == "INTTYPE");
//     REQUIRE(tokens[1][1].getToken() == "IDENTIFIER");
//     REQUIRE(tokens[1][1].getValue() == "id");

//     // Line 2: int age
//     REQUIRE(tokens[2][0].getToken() == "INTTYPE");
//     REQUIRE(tokens[2][1].getToken() == "IDENTIFIER");
//     REQUIRE(tokens[2][1].getValue() == "age");
// }

TEST_CASE("Lexer handles reference parameters in function signatures", "[lexer]")
{
    std::string source =
        "func increment(ref int val):\n"
        "    val = val + 1";

    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 2);
    // Line 0: func increment(ref int val):
    REQUIRE(tokens[0][0].getToken() == "FUNC");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "increment");
    REQUIRE(tokens[0][2].getToken() == "LPAREN");
    REQUIRE(tokens[0][3].getToken() == "REF");
    REQUIRE(tokens[0][4].getToken() == "INTTYPE");
    REQUIRE(tokens[0][5].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][5].getValue() == "val");
    REQUIRE(tokens[0][6].getToken() == "RPAREN");
    REQUIRE(tokens[0][7].getToken() == "COLON");
}

TEST_CASE("Lexer tokenizes explicit type cast function calls", "[lexer]")
{
    std::string source = "int converted = toInt(45.8) + toInt(12.3)";

    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 1);
    REQUIRE(tokens[0][0].getToken() == "INTTYPE");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "converted");
    REQUIRE(tokens[0][2].getToken() == "ASSIGN");

    // First cast: toInt(45.8)
    REQUIRE(tokens[0][3].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][4].getToken() == "LPAREN");
    REQUIRE(tokens[0][5].getToken() == "DECIMAL");
    REQUIRE(tokens[0][5].getValue() == "45.8");
    REQUIRE(tokens[0][6].getToken() == "RPAREN");

    REQUIRE(tokens[0][7].getToken() == "PLUS");

    // Second cast: toInt(12.3)
    REQUIRE(tokens[0][8].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][9].getToken() == "LPAREN");
    REQUIRE(tokens[0][10].getToken() == "DECIMAL");
    REQUIRE(tokens[0][10].getValue() == "12.3");
    REQUIRE(tokens[0][11].getToken() == "RPAREN");
}

TEST_CASE("Lexer tokenizes nested module scoping with double colon operator", "[lexer]")
{
    std::string source = "M::Vector v = M::Vector(10, 20)";

    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 1);
    REQUIRE(tokens[0][0].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][0].getValue() == "M");
    REQUIRE(tokens[0][1].getToken() == "DOUBLECOLON");
    REQUIRE(tokens[0][2].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][2].getValue() == "Vector");
    REQUIRE(tokens[0][3].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][3].getValue() == "v");
    REQUIRE(tokens[0][4].getToken() == "ASSIGN");
    REQUIRE(tokens[0][5].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][5].getValue() == "M");
    REQUIRE(tokens[0][6].getToken() == "DOUBLECOLON");
    REQUIRE(tokens[0][7].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][7].getValue() == "Vector");
}

TEST_CASE("Lexer tokenizes module exports and import statements with aliases", "[lexer]")
{
    std::string source =
        "export { Vector, scale }\n"
        "import math.geometry as Geo";

    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 2);
    // Line 0: export { Vector, scale }
    REQUIRE(tokens[0][0].getToken() == "EXPORT");
    REQUIRE(tokens[0][1].getToken() == "LCURLYBRACKET");
    REQUIRE(tokens[0][2].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][2].getValue() == "Vector");
    REQUIRE(tokens[0][3].getToken() == "COMMA");
    REQUIRE(tokens[0][4].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][4].getValue() == "scale");
    REQUIRE(tokens[0][5].getToken() == "RCURLYBRACKET");

    // Line 1: import math.geometry as Geo
    REQUIRE(tokens[1][0].getToken() == "IMPORT");
    REQUIRE(tokens[1][1].getToken() == "MODULE");
    REQUIRE(tokens[1][1].getValue() == "math.geometry");
    REQUIRE(tokens[1][2].getToken() == "AS");
    REQUIRE(tokens[1][3].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][3].getValue() == "Geo");
}

TEST_CASE("Lexer tokenizes while loops with comparison operators", "[lexer]")
{
    std::string source =
        "while i <= n:\n"
        "    result = result * i";

    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 2);
    // Line 0: while i <= n:
    REQUIRE(tokens[0][0].getToken() == "WHILE");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "i");
    REQUIRE(tokens[0][2].getToken() == "LESSTHANEQ"); // Less-than-equal (<=)
    REQUIRE(tokens[0][3].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][3].getValue() == "n");
    REQUIRE(tokens[0][4].getToken() == "COLON");

    // Line 1: result = result * i
    REQUIRE(tokens[1][0].getToken() == "INDENT");
    REQUIRE(tokens[1][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][1].getValue() == "result");
    REQUIRE(tokens[1][2].getToken() == "ASSIGN");
    REQUIRE(tokens[1][3].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][3].getValue() == "result");
    REQUIRE(tokens[1][4].getToken() == "STAR");
    REQUIRE(tokens[1][5].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][5].getValue() == "i");
}

TEST_CASE("Lexer tokenizes complex boolean logic with unary negation and comparison", "[lexer]")
{
    std::string source = "bool condition = !toBool(0) == (x >= 10)";

    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 1);
    REQUIRE(tokens[0][0].getToken() == "BOOLTYPE");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "condition");
    REQUIRE(tokens[0][2].getToken() == "ASSIGN");
    REQUIRE(tokens[0][3].getToken() == "NOT"); // Unary prefix !
    REQUIRE(tokens[0][4].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][5].getToken() == "LPAREN");
    REQUIRE(tokens[0][6].getToken() == "INT");
    REQUIRE(tokens[0][6].getValue() == "0");
    REQUIRE(tokens[0][7].getToken() == "RPAREN");
    REQUIRE(tokens[0][8].getToken() == "EQUALS"); // Equality (==)
    REQUIRE(tokens[0][9].getToken() == "LPAREN");
    REQUIRE(tokens[0][10].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][10].getValue() == "x");
    REQUIRE(tokens[0][11].getToken() == "GREATERTHANEQ"); // Greater-than-equal (>=)
    REQUIRE(tokens[0][12].getToken() == "INT");
    REQUIRE(tokens[0][12].getValue() == "10");
    REQUIRE(tokens[0][13].getToken() == "RPAREN");
}

TEST_CASE("Lexer handles type inference via 'var' keyword", "[lexer]")
{
    std::string source = "var inferredVal = -99.9";

    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 1);
    REQUIRE(tokens[0][0].getToken() == "VAR");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "inferredVal");
    REQUIRE(tokens[0][2].getToken() == "ASSIGN");
    REQUIRE(tokens[0][3].getToken() == "DECIMAL");
    REQUIRE(tokens[0][3].getValue() == "-99.9");
}

TEST_CASE("Lexer tokenizes container field accesses and mutations", "[lexer]")
{
    std::string source =
        "Person p = Person(1, 30)\n"
        "p.age = p.age + 1";

    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 2);
    // Line 0: Person p = Person(1, 30)
    REQUIRE(tokens[0][0].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][0].getValue() == "Person");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "p");
    REQUIRE(tokens[0][2].getToken() == "ASSIGN");
    REQUIRE(tokens[0][3].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][3].getValue() == "Person");

    // Line 1: p.age = p.age + 1
    REQUIRE(tokens[1][0].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][0].getValue() == "p");
    REQUIRE(tokens[1][1].getToken() == "DOT");
    REQUIRE(tokens[1][2].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][2].getValue() == "age");
    REQUIRE(tokens[1][3].getToken() == "ASSIGN");
    REQUIRE(tokens[1][4].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][4].getValue() == "p");
    REQUIRE(tokens[1][5].getToken() == "DOT");
    REQUIRE(tokens[1][6].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][6].getValue() == "age");
    REQUIRE(tokens[1][7].getToken() == "PLUS");
    REQUIRE(tokens[1][8].getToken() == "INT");
    REQUIRE(tokens[1][8].getValue() == "1");
}

TEST_CASE("Lexer handles multi-line top-level execution scripts cleanly", "[lexer]")
{
    std::string source =
        "int val = 5\n"
        "decimal res = toDecimal(val) * 2.5\n"
        "print(res)";

    lexer.tokenizeFile(source);
    auto tokens = lexer.getTokens();

    REQUIRE(tokens.size() >= 3);

    // Line 0: int val = 5
    REQUIRE(tokens[0][0].getToken() == "INTTYPE");
    REQUIRE(tokens[0][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[0][1].getValue() == "val");
    REQUIRE(tokens[0][2].getToken() == "ASSIGN");
    REQUIRE(tokens[0][3].getToken() == "INT");
    REQUIRE(tokens[0][3].getValue() == "5");

    // Line 1: decimal res = toDecimal(val) * 2.5
    REQUIRE(tokens[1][0].getToken() == "DECIMALTYPE");
    REQUIRE(tokens[1][1].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][1].getValue() == "res");
    REQUIRE(tokens[1][2].getToken() == "ASSIGN");
    REQUIRE(tokens[1][3].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][4].getToken() == "LPAREN");
    REQUIRE(tokens[1][5].getToken() == "IDENTIFIER");
    REQUIRE(tokens[1][5].getValue() == "val");
    REQUIRE(tokens[1][6].getToken() == "RPAREN");
    REQUIRE(tokens[1][7].getToken() == "STAR");
    REQUIRE(tokens[1][8].getToken() == "DECIMAL");
    REQUIRE(tokens[1][8].getValue() == "2.5");

    // Line 2: print(res)
    REQUIRE(tokens[2][0].getToken() == "IDENTIFIER");
    REQUIRE(tokens[2][1].getToken() == "LPAREN");
    REQUIRE(tokens[2][2].getToken() == "IDENTIFIER");
    REQUIRE(tokens[2][2].getValue() == "res");
    REQUIRE(tokens[2][3].getToken() == "RPAREN");
}
