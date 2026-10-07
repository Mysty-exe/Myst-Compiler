#include "compiler/lexer.h"

bool validIdentifier(const std::string &identifier)
{
    for (int i = 0; i < identifier.size(); i++)
    {
        if (!std::isalpha(identifier[i]) && identifier[i] != '_' && !(std::isdigit(identifier[i]) && i > 0))
            return false;
    }
    return true;
}

bool validNumber(const std::string &number)
{
    bool decimal = false;
    for (int i = 0; i < number.size(); i++)
    {
        if (number[i] == '.')
        {
            if (decimal)
                return false;
            decimal = true;
        }

        if (!(number[i] >= '0' && number[i] <= '9'))
            return false;
    }
    return true;
}

const std::vector<std::string> Token::keywords = {"func", "struct", "const", "var", "if", "elseif", "while", "break", "return", "import", "and", "or", "not"};
const std::vector<std::string> Token::dataTypes = {"string", "char", "int", "decimal", "bool"};
const std::vector<std::string> Token::operators = {"+", "-", "*", "/", "<", ">", "!", "==", "<=", ">=", "&&", "||", "and", "or", "not"};
const std::vector<std::string> Token::oneCharOperators = {"=", "+", "-", "*", "/", ":", "<", ">", "(", ")", ",", ".", "!"};
const std::vector<std::string> Token::twoCharOperators = {"==", "<=", ">=", "&&", "||", "->"};

Token::Token()
{
}

Token::Token(std::string value, int line, int col)
{
    this->value = value;
    this->lineNumber = line;
    this->columnNumber = col;

    if (value.size() > 1 && value.substr(0, 2) == "//")
        type = TokenType::COMMENT;
    else if (value.size() == 3 && value[0] == '\'' && value[value.size() - 1] == '\'')
    {
        value = value[1];
        type = TokenType::CHAR;
    }
    else if ((value[0] == '\"' && value[value.size() - 1] == '\"'))
    {
        value = value.substr(1, value.size() - 1);
        type = TokenType::STRING;
    }
    else if (validNumber(value))
    {
        if (std::count(value.begin(), value.end(), '.') == 1)
            type = TokenType::DECIMAL;
        else if (std::count(value.begin(), value.end(), '.') == 0)
            type = TokenType::INT;
    }
    else if (value == "true" || value == "false")
        type = TokenType::BOOL;
    else if (value == "char")
        type = TokenType::CHARTYPE;
    else if (value == "string")
        type = TokenType::STRINGTYPE;
    else if (value == "int")
        type = TokenType::INTTYPE;
    else if (value == "decimal")
        type = TokenType::DECIMALTYPE;
    else if (value == "bool")
        type = TokenType::BOOLTYPE;
    else if (value == "func")
        type = TokenType::FUNC;
    else if (value == "var")
        type = TokenType::VAR;
    else if (value == "if")
        type = TokenType::IF;
    else if (value == "elseif")
        type = TokenType::ELSEIF;
    else if (value == "else")
        type = TokenType::ELSE;
    else if (value == "while")
        type = TokenType::WHILE;
    else if (value == "for")
        type = TokenType::FOR;
    else if (value == "break")
        type = TokenType::BREAK;
    else if (value == "return")
        type = TokenType::RETURN;
    else if (value == "=")
        type = TokenType::ASSIGN;
    else if (value == "+")
        type = TokenType::PLUS;
    else if (value == "-")
        type = TokenType::MINUS;
    else if (value == "*")
        type = TokenType::STAR;
    else if (value == "/")
        type = TokenType::SLASH;
    else if (value == "!=")
        type = TokenType::NEQUALS;
    else if (value == "==")
        type = TokenType::EQUALS;
    else if (value == "<")
        type = TokenType::LESSTHAN;
    else if (value == ">")
        type = TokenType::GREATERTHAN;
    else if (value == "<=")
        type = TokenType::LESSTHANEQ;
    else if (value == ">=")
        type = TokenType::GREATERTHANEQ;
    else if (value == "and" || value == "&&")
        type = TokenType::AND;
    else if (value == "or" || value == "||")
        type = TokenType::OR;
    else if (value == "not" || value == "!")
        type = TokenType::NOT;
    else if (value == ":")
        type = TokenType::COLON;
    else if (value == "->")
        type = TokenType::ARROW;
    else if (value == "(")
        type = TokenType::LPAREN;
    else if (value == ")")
        type = TokenType::RPAREN;
    else if (value == ",")
        type = TokenType::COMMA;
    else if (value == ".")
        type = TokenType::DOT;
    else if (value == "\n")
        type = TokenType::NEWLINE;
    else if (value == "\t")
        type = TokenType::INDENT;
    else if (validIdentifier(value))
        type = TokenType::IDENTIFIER;
    else
        type = TokenType::UNKNOWN;
}

TokenType Token::getTokenDataType(std::string type)
{
    if (type == "CHARTYPE")
        return TokenType::CHARTYPE;
    if (type == "STRINGTYPE")
        return TokenType::STRINGTYPE;
    if (type == "INTTYPE")
        return TokenType::INTTYPE;
    if (type == "DECIMALTYPE")
        return TokenType::DECIMALTYPE;
    if (type == "BOOLTYPE")
        return TokenType::BOOLTYPE;
}

std::string Token::getType(TokenType type)
{
    switch (type)
    {
    case TokenType::COMMENT:
        return "COMMENT";
    case TokenType::CHAR:
        return "CHAR";
    case TokenType::STRING:
        return "STRING";
    case TokenType::INT:
        return "INT";
    case TokenType::DECIMAL:
        return "DECIMAL";
    case TokenType::BOOL:
        return "BOOL";
    case TokenType::CHARTYPE:
        return "CHARTYPE";
    case TokenType::STRINGTYPE:
        return "STRINGTYPE";
    case TokenType::INTTYPE:
        return "INTTYPE";
    case TokenType::DECIMALTYPE:
        return "DECIMALTYPE";
    case TokenType::BOOLTYPE:
        return "BOOLTYPE";
    case TokenType::FUNC:
        return "FUNC";
    case TokenType::VAR:
        return "VAR";
    case TokenType::IF:
        return "IF";
    case TokenType::ELSEIF:
        return "ELSEIF";
    case TokenType::ELSE:
        return "ELSE";
    case TokenType::WHILE:
        return "WHILE";
    case TokenType::FOR:
        return "FOR";
    case TokenType::BREAK:
        return "BREAK";
    case TokenType::RETURN:
        return "RETURN";
    case TokenType::ASSIGN:
        return "ASSIGN";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::MINUS:
        return "MINUS";
    case TokenType::STAR:
        return "STAR";
    case TokenType::SLASH:
        return "SLASH";
    case TokenType::EQUALS:
        return "EQUALS";
    case TokenType::NEQUALS:
        return "NEQUALS";
    case TokenType::LESSTHAN:
        return "LESSTHAN";
    case TokenType::GREATERTHAN:
        return "GREATERTHAN";
    case TokenType::AND:
        return "AND";
    case TokenType::OR:
        return "OR";
    case TokenType::NOT:
        return "NOT";
    case TokenType::COLON:
        return "COLON";
    case TokenType::ARROW:
        return "ARROW";
    case TokenType::LPAREN:
        return "LPAREN";
    case TokenType::RPAREN:
        return "RPAREN";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::DOT:
        return "DOT";
    case TokenType::NEWLINE:
        return "NEWLINE";
    case TokenType::INDENT:
        return "INDENT";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    default:
        return "UNKNOWN";
    }
}

std::string Token::getToken() const
{
    return Token::getType(type);
}

std::string Token::getValue() const
{
    return value;
}

int Token::getLineNumber() const
{
    return lineNumber;
}

Lexer::Lexer()
{
}

void Lexer::addTabsInLine(const std::string &line, int lineNum, int &col)
{
    for (int i = 0; i < line.size(); i++)
    {
        if (line[i] == '\t')
            tokens[tokens.size() - 1].push_back(Token("\t", lineNum, col + 1));
        else
            break;

        col++;
    }
}

bool Lexer::validNumber(const std::string &str) const
{
    if (str.empty())
        return false;
    try
    {
        size_t pos;
        double test = std::stod(str, &pos);
        return pos == str.size();
    }
    catch (const std::out_of_range &)
    {
        return false;
    }
    catch (const std::invalid_argument &)
    {
        return false;
    }
}

int Lexer::countSubstrings(const std::string &str, const std::string &substr) const
{
    if (substr.empty())
        return 0;
    int count = 0;
    size_t pos = 0;
    while ((pos = str.find(substr, pos)) != std::string::npos)
    {
        ++count;
        pos += substr.length();
    }
    return count;
}

bool Lexer::isNegativeNumber(const std::string &line, int col) const
{
    return (line.size() - col > 1 && line[col] == '-' && std::isdigit(line[col + 1]));
}

bool Lexer::startedNewString(const std::string &line, int col) const
{
    return line[col] == '\"' || line[col] == '\'';
}

bool Lexer::stringHasntEnded(bool inString, const std::string &line, int col) const
{
    return inString && !startedNewString(line, col);
}

bool Lexer::commentStarted(const std::string &line, int col) const
{
    return line.size() - col > 1 && line.substr(col, 2) == "//";
}

bool Lexer::isAOneCharOperator(const std::string &line, int col) const
{
    return std::find(Token::oneCharOperators.begin(), Token::oneCharOperators.end(), std::string() + line[col]) != Token::oneCharOperators.end();
}

bool Lexer::isATwoCharOperator(const std::string &line, int col) const
{
    return line.size() - col > 1 && std::find(Token::twoCharOperators.begin(), Token::twoCharOperators.end(), line.substr(col, 2)) != Token::twoCharOperators.end();
}

void Lexer::addCurrentToken(std::string &token, int lineNum, int col)
{
    if (token.size() > 0)
    {
        Token t = Token(token, lineNum, col);
        tokens[tokens.size() - 1].push_back(t);
        token = "";
    }
}

void Lexer::tokenizeFile(std::ifstream &file)
{
    tokens.clear();

    int lineNum = 0;
    std::string line, currentToken;
    bool inString = false;

    while (std::getline(file, line))
    {
        tokens.push_back({});
        int col = 0;
        addTabsInLine(line, lineNum + 1, col);

        for (col; col < line.size(); col++)
        {
            if (stringHasntEnded(inString, line, col))
            {
                currentToken += line[col];
                continue;
            }

            if (commentStarted(line, col))
            {
                addCurrentToken(currentToken, lineNum + 1, col + 1);
                break;
            }

            if (line[col] == ' ')
            {
                addCurrentToken(currentToken, lineNum + 1, col + 1);
                continue;
            }

            if (startedNewString(line, col))
            {
                if (!inString)
                    addCurrentToken(currentToken, lineNum + 1, col + 1);

                currentToken += line[col];
                inString = !inString;
                if (!inString)
                    addCurrentToken(currentToken, lineNum + 1, col + 1);
                continue;
            }

            if (isATwoCharOperator(line, col))
            {
                addCurrentToken(currentToken, lineNum + 1, col + 1);

                tokens[tokens.size() - 1].push_back(Token(line.substr(col, 2), lineNum + 1, line.size()));
                col += 1;
                continue;
            }

            if (isAOneCharOperator(line, col))
            {
                if (!isNegativeNumber(line, col))
                {
                    addCurrentToken(currentToken, lineNum + 1, col + 1);
                    tokens[tokens.size() - 1].push_back(Token(std::string() + line[col], lineNum + 1, line.size()));
                    continue;
                }
            }

            currentToken += line[col];
        };

        addCurrentToken(currentToken, lineNum + 1, col + 1);
        inString = false;

        tokens[tokens.size() - 1].push_back(Token("\n", lineNum + 1, line.size()));
        lineNum += 1;
    }

    tokens.push_back({Token("\n", lineNum, line.size())});
    file.close();
}

void Lexer::readTokens() const
{
    for (auto &line : tokens)
    {
        for (const Token &token : line)
        {
            if (token.getToken() != "NEWLINE")
                std::cout << token.getToken() << " ";
        }
        std::cout << std::endl;
    }
}

void Lexer::readFile() const
{
    for (auto &line : tokens)
    {
        for (const Token &token : line)
        {
            if (token.getToken() != "NEWLINE")
                std::cout << token.getValue() << " ";
        }
        std::cout << std::endl;
    }
}

std::vector<std::vector<Token>> Lexer::getTokens() const
{
    return tokens;
};
