#pragma once
#include <bits/stdc++.h>
#include "Symbol.h"

// 前向声明
class Scanner;
class Parser;
class Interpreter;
class Table;

#define norw 10       // number of reserved words
#define txmax_val 100     // maxmium size of symbol table
#define smax_val 100      // maxmium length of a string
#define nmax 14       // maxmium number of digits of a number
#define max_length 10 // maxmium length of an identifier
#define maxerr 30     // maxmium number of errors
#define amax 2048     // upper bound of address

#define levmax 3      // maxmium number of nested declaration levels
#define cxmax 300     // maxmium number of VMcode's rows
#define symnum 33     // number of symbols


using namespace std;

// 全局变量声明
extern bool listswitch;  // 是否列出代码
extern bool tableswitch; // 是否列出符号表
extern ostringstream fas_stream;     // 一般输出文件流
extern ostringstream fcode_stream;   // 指令代码输出文件流
extern ostringstream ftable_stream;  // 符号表输出文件流
extern ostringstream fresult_stream; // 程序执行结果输出文件流

class Compiler
{
private:
    /* data */
public:
    Table *table;
    Parser *parser;
    Interpreter *interp;
    Scanner *lex;   // 词法分析器  
    istringstream* source_stream;  
    bool compile();
    bool execute(); // 执行生成的指令
    Compiler(const std::string& source_code); 
    ~Compiler();
};
