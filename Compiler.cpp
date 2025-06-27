#include "Compiler.hpp"
#include "Scanner.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"
#include "table.hpp"

// 全局变量定义
bool listswitch = false;
bool tableswitch = false;
extern ofstream fas;
extern ofstream fcode;
extern ofstream ftable;
extern ofstream fresult;

// 构造函数
Compiler::Compiler(ifstream &fin)
{
    interp = new Interpreter();     //解释器
    lex = new Scanner(fin);         //词法分析器
    table = new Table(*lex);        //符号表
    parser = new Parser(*lex, *table, *interp);     //语法分析器+目标代码生成
}


Compiler::~Compiler()
{
    delete table;       //符号表
    delete parser;      //语法分析器+目标代码生成
    delete interp;      //解释器
    delete lex;
}

bool Compiler::compile()
{
    try
    {
        parser->parse(); // 开始解析

        // 输出P-code指令
        if (listswitch)
        {
            cout << "\n=== Generated P-Code Instructions ===" << endl;
            fas << "\n=== Generated P-Code Instructions ===" << endl;
            fcode << "\n=== Generated P-Code Instructions ===" << endl;
            interp->listCode(0);
        }

        // 输出符号表
        if (tableswitch)
        {
            cout << "\n=== Symbol Table ===" << endl;
            fas << "\n=== Symbol Table ===" << endl;
            ftable << "\n=== Symbol Table ===" << endl;
            table->debugTable(0);
        }

        // 输出编译结果
        int errorCount = Parser::errorCount;
        if (errorCount == 0)
        {
            cout << "\n===Parsing success!===" << endl;
            fas << "\n===Parsing success!===" << endl;
            fcode << "\n===Parsing success!===" << endl;
            ftable << "\n===Parsing success!===" << endl;
        }
        else
        {
            cout << "\n"
                 << errorCount << " errors in L25 program!" << endl;
            fas << "\n"
                 << errorCount << " errors in L25 program!" << endl;
            fcode << "\n"
                 << errorCount << " errors in L25 program!" << endl;
            ftable << "\n"
                 << errorCount << " errors in L25 program!" << endl;
        }

        return errorCount == 0;
    }
    catch (const std::exception &e)
    {
        cerr << "Compilation error: " << e.what() << endl;
        fas << "Compilation error: " << e.what() << endl;
        fcode << "Compilation error: " << e.what() << endl;
        ftable << "Compilation error: " << e.what() << endl;
        return false;
    }
}

bool Compiler::execute()
{
    try
    {
        int errorCount = Parser::errorCount;
        if (errorCount == 0){
            cout << "\n=== Executing Program ===" << endl;
            fas << "\n=== Executing Program ===" << endl;
            fresult << "\n=== Executing Program ===" << endl;
            interp->interpret();
            cout << "\n=== Execution Completed ===" << endl;
            fas << "\n=== Execution Completed ===" << endl;
            fresult << "\n=== Execution Completed ===" << endl;
            return true;
        }else{
            cout << "Cannot execute: compilation errors exist!" << endl;
            fas << "Cannot execute: compilation errors exist!" << endl;
            fresult << "Cannot execute: compilation errors exist!" << endl;
            return false;
        }
    }catch (const std::exception &e){
        cerr << "Execution error: " << e.what() << endl;
        fas << "Execution error: " << e.what() << endl;
        fresult << "Execution error: " << e.what() << endl;
        return false;
    }
}
