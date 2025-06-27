#include "Compiler.hpp"
#include "Scanner.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"
#include "table.hpp"

// 全局变量定义
// bool listswitch = false;
// bool tableswitch = false;
extern ostringstream fas_stream;
extern ostringstream fcode_stream;
extern ostringstream ftable_stream;
extern ostringstream fresult_stream;

// 构造函数
Compiler::Compiler(const std::string& source_code)
{
    // 从源代码字符串创建一个内存输入流
    source_stream = new istringstream(source_code);

    interp = new Interpreter();
    lex = new Scanner(*source_stream); // 将内存流传递给词法分析器
    table = new Table(*lex);
    parser = new Parser(*lex, *table, *interp);
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
            fas_stream << "\n=== Generated P-Code Instructions ===" << endl;
            fcode_stream << "\n=== Generated P-Code Instructions ===" << endl;
            interp->listCode(0);
        }

        // 输出符号表
        if (tableswitch)
        {
            cout << "\n=== Symbol Table ===" << endl;
            fas_stream << "\n=== Symbol Table ===" << endl;
            ftable_stream << "\n=== Symbol Table ===" << endl;
            table->debugTable(0);
        }

        // 输出编译结果
        int errorCount = Parser::errorCount;
        if (errorCount == 0)
        {
            cout << "\n===Parsing success!===" << endl;
            fas_stream << "\n===Parsing success!===" << endl;
            fcode_stream << "\n===Parsing success!===" << endl;
            ftable_stream << "\n===Parsing success!===" << endl;
        }
        else
        {
            cout << "\n"
                 << errorCount << " errors in L25 program!" << endl;
            fas_stream << "\n"
                 << errorCount << " errors in L25 program!" << endl;
            fcode_stream << "\n"
                 << errorCount << " errors in L25 program!" << endl;
            ftable_stream << "\n"
                 << errorCount << " errors in L25 program!" << endl;
        }

        return errorCount == 0;
    }
    catch (const std::exception &e)
    {
        cerr << "Compilation error: " << e.what() << endl;
        fas_stream << "Compilation error: " << e.what() << endl;
        fcode_stream << "Compilation error: " << e.what() << endl;
        ftable_stream << "Compilation error: " << e.what() << endl;
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
            fas_stream << "\n=== Executing Program ===" << endl;
            fresult_stream << "\n=== Executing Program ===" << endl;
            interp->interpret();
            cout << "\n=== Execution Completed ===" << endl;
            fas_stream << "\n=== Execution Completed ===" << endl;
            fresult_stream << "\n=== Execution Completed ===" << endl;
            return true;
        }else{
            cout << "Cannot execute: compilation errors exist!" << endl;
            fas_stream << "Cannot execute: compilation errors exist!" << endl;
            fresult_stream << "Cannot execute: compilation errors exist!" << endl;
            return false;
        }
    }catch (const std::exception &e){
        cerr << "Execution error: " << e.what() << endl;
        fas_stream << "Execution error: " << e.what() << endl;
        fresult_stream << "Execution error: " << e.what() << endl;
        return false;
    }
}
