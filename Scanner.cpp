#include "Scanner.hpp"
#include "Compiler.hpp"
#include <iostream>

void Scanner::getch()
{
    // Function to read the next character from input
    if (char_position >= line_length)
    {
        if (input_file && getline(*input_file, line)){
            char_position = 0;
            line_length = line.length();
            line_number++;
            // 输出行号和源代码行到控制台和文件
            cout << line_number << " " << line << endl;
            if (fas.is_open()) fas << line_number << " " << line << endl;
        }
        else{
            ch = EOF;
            return;
        }
    }

    ch = char_position < line_length?line[char_position]:'\n';
    char_position++;
}

void Scanner::getsym()
{
    while (isspace(ch)) getch(); // Skip whitespace characters

    if (ch == EOF){
        sym = Symbol::nul;
        return;
    }
    if (isalpha(ch)||ch == '_') matchKeywordOrIdentifier(); // If the character is an alphabetic character
    else if (isdigit(ch)) matchNumber();
    else if (ch == '"') matchString(); // 匹配字符串字面量
    else matchOpertor();
}

void Scanner::matchKeywordOrIdentifier()
{
    // Match reserved words or identifiers
    id.clear();
    string original_id;
    while (isalnum(ch) || ch == '_')
    { // Allow alphanumeric characters and underscores
        original_id += ch;
        id += tolower(ch); // 转换为小写用于关键字匹配
        getch();
    }
    // Check if the identifier is a reserved word
    auto it = find(begin(word), end(word), id);
    if (it != end(word))
    {
        sym = wsym[it - begin(word)]; // Set the symbol to the corresponding reserved word symbol
    }
    else
    {
        id = original_id;    // 如果不是关键字，保持原始大小写
        sym = Symbol::ident; // Set the symbol to identifier
    }
}

void Scanner::matchNumber()
{
    // Match numbers
    int bitcnt = 0; // Bit count for numbers
    num = 0;
    while (isdigit(ch))
    {
        num = num * 10 + (ch - '0'); // Convert character to integer
        getch();
        bitcnt++;
    }
    if (bitcnt > nmax)
    {
        cerr << "Error: Number too long" << endl; // Error if number exceeds length
    }
    sym = Symbol::number; // Set the symbol to number
}

void Scanner::matchString()
{
    str_val.clear();
    getch(); // 跳过开始的引号

    while (ch != '"' && ch != EOF)
    {
        if (ch == '\\')
        { // 处理转义字符
            getch();
            switch (ch)
            {
            case 'n':
                str_val += '\n';
                break;
            case 't':
                str_val += '\t';
                break;
            case '"':
                str_val += '"';
                break;
            case '\\':
                str_val += '\\';
                break;
            default:
                str_val += '\\';
                str_val += ch;
                break;
            }
        }
        else
        {
            str_val += ch;
        }
        getch();
    }

    if (ch == '"')
    {
        getch(); // 跳过结束的引号
        sym = Symbol::string_literal;
    }
    else
    {
        cerr << "Error: Unterminated string literal" << endl;
        sym = Symbol::nul;
    }
}

void Scanner::matchOpertor()
{
    switch (ch)
    {
    case '=':
        getch();
        if (ch == '=')
        {
            sym = Symbol::eql; // Equal operator ==
            getch();
        }
        else
        {
            sym = Symbol::becomes; // Assignment operator =
        }
        break;
    case '!':
        getch();
        if (ch == '=')
        {
            sym = Symbol::neq; // Not equal operator
            getch();
        }
        else
        {
            cerr << "Error: Expected '=' after '!'" << endl;
            sym = Symbol::nul;
        }
        break;
    case '<':
        getch();
        if (ch == '=')
        {
            sym = Symbol::leq; // Less than or equal operator
            getch();
        }
        else
        {
            sym = Symbol::lss; // Less than operator
        }
        break;
    case '>':
        getch();
        if (ch == '=')
        {
            sym = Symbol::geq; // Greater than or equal operator
            getch();
        }
        else
        {
            sym = Symbol::gtr; // Greater than operator
        }
        break;
    default:
        if (ssym[(unsigned char)ch] != Symbol::nul)
        {
            sym = ssym[(unsigned char)ch]; // Set the symbol to the corresponding single character symbol
            getch();
        }
        else
        {
            cerr << "Error: Unknown symbol '" << ch << "'" << endl; // Error for unknown symbols
            getch();                                                // Move to the next character
            sym = Symbol::nul;
        }
        break;
    }
}

Scanner::Scanner()
{
    input_file = nullptr;
    initializeSymbols();
}

Scanner::Scanner(ifstream &file)
{
    input_file = &file;
    initializeSymbols();
    // 读取第一个字符
    getch();
}

void Scanner::initializeSymbols()
{
    // 初始化所有符号为nul
    for (int i = 0; i < 256; i++)
    {
        ssym[i] = Symbol::nul;
    }
    ssym['+'] = Symbol::plus;
    ssym['-'] = Symbol::minus;
    ssym['*'] = Symbol::times;
    ssym['/'] = Symbol::divide;
    ssym['('] = Symbol::lparen;
    ssym[')'] = Symbol::rparen;
    ssym['{'] = Symbol::lbrace;
    ssym['}'] = Symbol::rbrace;
    ssym[','] = Symbol::comma;
    ssym[';'] = Symbol::semicolon;
    ssym['@'] = Symbol::atsym; // @符号
    ssym['&'] = Symbol::addresssym; // &符号
    // ssym['\''] = Symbol::quote;
    // ssym['\\'] = Symbol::backslash;
    // ssym['<'] = Symbol::lss;
    // ssym['>'] = Symbol::gtr;


    //关键字
    word[0] = "program";
    word[1] = "main";
    word[2] = "str";
    word[3] = "let";
    word[4] = "if";
    word[5] = "else";
    word[6] = "while";
    word[7] = "func";
    word[8] = "return";
    word[9] = "input";
    word[10] = "output";

    wsym[0] = Symbol::programsym;
    wsym[1] = Symbol::mainsym;
    wsym[2] = Symbol::strsym;
    wsym[3] = Symbol::letsym;
    wsym[4] = Symbol::ifsym;
    wsym[5] = Symbol::elsesym;
    wsym[6] = Symbol::whilesym;
    wsym[7] = Symbol::funcsym;
    wsym[8] = Symbol::returnsym;
    wsym[9] = Symbol::inputsym;
    wsym[10] = Symbol::outputsym; // 初始化成员变量
    line_length = 0;
    char_position = 0;
    line_number = 0; // 初始化行号
    sym = Symbol::nul;
    num = 0;
    ch = ' ';
}

Scanner::~Scanner()
{
    // Destructor
    // Clean up resources if needed
}
