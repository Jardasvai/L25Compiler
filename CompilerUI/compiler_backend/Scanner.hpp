#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <cctype>
#include <algorithm>
#include "Symbol.h"
using namespace std;

class Scanner
{
private:
    char ch;                                 // current character
    string line;
    string word[11];      // reserved words
    Symbol ssym[256];     // single character symbols
    Symbol wsym[11];      // reserved words symbols
    ifstream *input_file; // 输入文件流
    istream* input_stream;

public:
    int line_length;   // length of the current line
    int char_position; // position of current character in the line
    int line_number;   // current line number
    Symbol sym;        // current symbol
    string id;         // current identifier
    int num;           // current number
    string str_val;    // current string literal

    Scanner();
    Scanner(ifstream &file);
    Scanner(std::istream& stream);
    ~Scanner();
    void getch();
    void getsym();
    void matchKeywordOrIdentifier();
    void matchNumber();
    void matchOpertor();
    void matchString(); // 新增：匹配字符串字面量

private:
    void initializeSymbols(); // 初始化符号表
};
