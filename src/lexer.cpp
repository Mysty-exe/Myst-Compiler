#include <compiler/lexer.h>

Token::Token(std::string value, int line, int col)
{
    this->value = value;
    this->lineNumber = line;
    this->columnNumber = col;

    if (value.size() > 1 && value.compare(0, 2, "//") == 0)
    {
        type = COMMENT;
    }
    if (value[0] == '\"' || value[1] == '\'')
    {
        type = STRING;
    }
    else if (value == "func")
        type = FUNC;
    else if (value == "struct")
        type = STRUCT;
    else if (value == "const")
        type = CONST;
    else if (value == "var")
        type = VAR;
    else if (value == "if")
        type = IF;
    else if (value == "elseif")
        type = ELSEIF;
    else if (value == "else")
        type = ELSE;
    else if (value == "while")
        type = WHILE;
    else if (value == "break")
        type = BREAK;
    else if (value == "return")
        type = RETURN;
    else if (value == "import")
        type = IMPORT;
    else if (value == "import")
        type = IMPORT;
    else if (value == "=")
        type = ASSIGN;
    else if (value == "+")
        type = PLUS;
    else if (value == "-")
        type = MINUS;
    else if (value == "*")
        type = STAR;
    else if (value == "/")
        type = SLASH;
    else if (value == "==")
        type = EQUALS;
    else if (value == "<")
        type = LESSTHAN;
    else if (value == ">")
        type = GREATERTHAN;
    else if (value == "<=")
        type = LESSTHANEQ;
    else if (value == ">=")
        type = GREATERTHANEQ;
    else if (value == "and" || value == "&&")
        type = AND;
    else if (value == "or" || value == "||")
        type = OR;
    else if (value == ":")
        type = COLON;
    else if (value == "->")
        type = ARROW;
    else if (value == "(")
        type = LPAREN;
    else if (value == ")")
        type = RPAREN;
    else if (value == ",")
        type = COMMA;
    else if (value == ".")
        type = DOT;
    else if (value == "\n")
        type = NEWLINE;
    else if (value == "\t")
        type = INDENT;
    else if (std::isdigit(value[0]))
    {
        if (std::count(value.begin(), value.end(), '.') == 1)
            type = FLOAT;
        else if (std::count(value.begin(), value.end(), '.') == 1)
            type = INT;
    }
    else if (value == "")
        type == ENDFILE;
    else
        type == UNKNOWN;
}

Lexer::Lexer()
{
}

void Lexer::tokenizeFile(std::ifstream &file)
{
    tokens.clear();
}

std::vector<Token> Lexer::getTokens() const
{
    return tokens;
};
