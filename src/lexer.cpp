#include <compiler/lexer.h>

const std::vector<std::string> Token::keywords = {"func", "struct", "const", "var", "if", "elseif", "while", "break", "return", "import", "and", "or"};
const std::vector<std::string> Token::oneCharOperators = {"=", "+", "-", "*", "/", ":", "<", ">", "(", ")", ",", "."};
const std::vector<std::string> Token::twoCharOperators = {"==", "<=", ">=", "&&", "||", "->"};

Token::Token(std::string value, int line, int col)
{
    this->value = value;
    this->lineNumber = line;
    this->columnNumber = col;

    if (value.size() > 1 && value.substr(0, 2) == "//")
        type = COMMENT;
    else if (value[0] == '\"' || value[0] == '\'')
        type = STRING;
    else if (std::isdigit(value[0]) || (value.size() > 1 && value[0] == '-' && std::isdigit(value[1])))
    {
        if (std::count(value.begin(), value.end(), '.') == 1)
            type = FLOAT;
        else if (std::count(value.begin(), value.end(), '.') == 1)
            type = INT;
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
    else if (value == "for")
        type = FOR;
    else if (value == "break")
        type = BREAK;
    else if (value == "return")
        type = RETURN;
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
    else if (value == "")
        type = ENDFILE;
    else
        type = IDENTIFIER;
}

std::string Token::getToken() const
{
    switch (type)
    {
    case COMMENT:
        return "COMMENT";
    case STRING:
        return "STRING";
    case INT:
        return "INT";
    case FLOAT:
        return "FLOAT";
    case FUNC:
        return "FUNC";
    case STRUCT:
        return "STRUCT";
    case CONST:
        return "CONST";
    case VAR:
        return "VAR";
    case IF:
        return "IF";
    case ELSEIF:
        return "ELSEIF";
    case ELSE:
        return "ELSE";
    case WHILE:
        return "WHILE";
    case FOR:
        return "FOR";
    case BREAK:
        return "BREAK";
    case RETURN:
        return "RETURN";
    case IMPORT:
        return "IMPORT";
    case ASSIGN:
        return "ASSIGN";
    case PLUS:
        return "PLUS";
    case MINUS:
        return "MINUS";
    case STAR:
        return "STAR";
    case SLASH:
        return "SLASH";
    case EQUALS:
        return "EQUALS";
    case LESSTHAN:
        return "LESSTHAN";
    case GREATERTHAN:
        return "GREATERTHAN";
    case AND:
        return "AND";
    case OR:
        return "OR";
    case COLON:
        return "COLON";
    case ARROW:
        return "ARROW";
    case LPAREN:
        return "LPAREN";
    case RPAREN:
        return "RPAREN";
    case COMMA:
        return "COMMA";
    case DOT:
        return "DOT";
    case NEWLINE:
        return "NEWLINE";
    case INDENT:
        return "INDENT";
    case ENDFILE:
        return "ENDFILE";
    default:
        return "IDENTIFIER";
    }
}

std::string Token::getValue() const
{
    return value;
}

Lexer::Lexer()
{
}

void Lexer::addTabsInLine(const std::string &line, int lineNum, int &col)
{
    while (line.size() - col > 4)
    {
        if (line.substr(col, 4) == "    ")
            tokens.push_back(Token("\t", lineNum, col));
        else
            break;

        col += 4;
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
        tokens.push_back(Token(token, lineNum + 1, col + 1));
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
        int col = 0;
        addTabsInLine(line, lineNum, col);

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
                tokens.push_back(Token(line.substr(col), lineNum + 1, col + 1));
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

                tokens.push_back(Token(line.substr(col, 2), lineNum + 1, line.size()));
                col += 1;
                continue;
            }

            if (isAOneCharOperator(line, col))
            {
                if (!isNegativeNumber(line, col) && !validNumber(currentToken))
                {
                    addCurrentToken(currentToken, lineNum + 1, col + 1);

                    tokens.push_back(Token(std::string() + line[col], lineNum + 1, line.size()));
                    continue;
                }
            }

            currentToken += line[col];
        };

        addCurrentToken(currentToken, lineNum + 1, col + 1);
        inString = false;

        tokens.push_back(Token("\n", lineNum + 1, line.size()));
        lineNum += 1;
    }

    tokens.pop_back();
    tokens.push_back(Token("", lineNum, line.size()));
    file.close();
}

void Lexer::readTokens() const
{
    for (const Token &token : tokens)
    {
        std::cout << token.getToken() << " ";
        if (token.getToken() == "NEWLINE")
            std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Lexer::readFile() const
{
    for (const Token &token : tokens)
        std::cout << token.getValue() << " ";
    std::cout << std::endl;
}

std::vector<Token> Lexer::getTokens() const
{
    return tokens;
};
