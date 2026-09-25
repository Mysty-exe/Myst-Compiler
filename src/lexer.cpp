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

bool Lexer::validNumber(const std::string &str)
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

int Lexer::countSubstrings(const std::string &str, const std::string &substr)
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

void Lexer::tokenizeFile(std::ifstream &file)
{
    tokens.clear();

    int lineNum = 0;
    std::string line, currentToken;
    bool inString = false;

    while (std::getline(file, line))
    {
        int tabStart = 0;
        size_t tabPos = line.find("    ");

        if (tabPos != std::string::npos)
        {
            for (int i = 0; i < countSubstrings(line, "    "); i++)
            {
                tokens.push_back(Token("\t", lineNum + 1, tabStart + 1));
                tabStart += 4;
            }
        }

        for (int col = tabStart; col < line.size(); col++)
        {
            if (inString && !(line[col] == '\"' || line[col] == '\''))
            {
                currentToken += line[col];
                continue;
            }

            if (line.size() - col > 1 && line.substr(col, 2) == "//")
            {
                if (currentToken.size() > 0)
                {
                    tokens.push_back(Token(currentToken, lineNum + 1, line.size()));
                    currentToken = "";
                }

                tokens.push_back(Token(line.substr(col), lineNum + 1, col + 1));
                break;
            }

            if (line[col] == ' ')
            {
                if (currentToken.size() > 0)
                {
                    tokens.push_back(Token(currentToken, lineNum + 1, line.size()));
                    currentToken = "";
                }

                continue;
            }

            if (line[col] == '\"' || line[col] == '\'')
            {
                if (!inString)
                {
                    if (currentToken.size() > 0)
                    {
                        tokens.push_back(Token(currentToken, lineNum + 1, line.size()));
                        currentToken = "";
                    }
                }

                currentToken += line[col];
                inString = !inString;
                if (!inString)
                {
                    tokens.push_back(Token(currentToken, lineNum + 1, line.size()));
                    currentToken = "";
                }

                continue;
            }

            if (line.size() - col > 1 && std::find(Token::twoCharOperators.begin(), Token::twoCharOperators.end(), line.substr(col, 2)) != Token::twoCharOperators.end())
            {
                if (currentToken.size() > 0)
                    tokens.push_back(Token(currentToken, lineNum + 1, line.size()));

                tokens.push_back(Token(line.substr(col, 2), lineNum + 1, line.size()));
                col += 1;
                continue;
            }

            if (std::find(Token::oneCharOperators.begin(), Token::oneCharOperators.end(), std::string() + line[col]) != Token::oneCharOperators.end())
            {
                if (!(line.size() - col > 1 && line[col] == '-' && std::isdigit(line[col + 1])) && !validNumber(currentToken))
                {
                    if (currentToken.size() > 0)
                    {
                        tokens.push_back(Token(currentToken, lineNum + 1, line.size()));
                        currentToken = "";
                    }

                    tokens.push_back(Token(std::string() + line[col], lineNum + 1, line.size()));
                    continue;
                }
            }

            currentToken += line[col];
        };

        if (currentToken.size() > 0)
        {
            tokens.push_back(Token(currentToken, lineNum + 1, line.size()));
            inString = false;
            currentToken = "";
        }
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
}

void Lexer::readTokenValues() const
{
    for (const Token &token : tokens)
        std::cout << token.getValue() << " ";
    std::cout << std::endl;
}

std::vector<Token> Lexer::getTokens() const
{
    return tokens;
};
