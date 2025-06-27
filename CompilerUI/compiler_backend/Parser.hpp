#pragma once
#include "Symbol.h"
#include "SymSet.hpp"

// 前向声明
class Scanner;
class Table;
class Interpreter;

class Parser
{
private:
    Scanner &lex;                                  // 对词法分析器引用
    Table &table;                                  // 对符号表引用
    Interpreter &interp;                           // 对目标代码生成器的引用
    static const int sym_num = (int)Symbol::COUNT; // 符号数量
    // 声明开始符号集合，表示语句开始的符号集合，表示因子开始的符号集合
    SymSet declbegsys, statbegsys, facbegsys;
    Symbol sym; // 当前符号
    int dx = 0; // 当前作用域的堆栈帧大小，或者说数据大小（data size）
public:
    static int errorCount; // 静态错误计数器
    int argsToClean =0; // 函数调用后需要清理的参数数量
    Parser(Scanner &scanner, Table &symbolTable, Interpreter &interpreter);
    ~Parser();
    void parse();                                                // 解析入口函数
    void parseProgram(int lev, SymSet &fsys);                    // 解析程序
    void parseFunctionDef(SymSet &fsys);                         // 解析函数定义
    void parseParameterList(SymSet &fsys);                       // 解析参数列表
    void parseStatementList(SymSet &fsys, int lev = 0);          // 解析语句列表
    void nextSym();                                              // 获取下一个符号
    void test(SymSet &s1, SymSet &s2, int n);                    // 测试符号集合
    void error(int n);                                           // 错误处理函数                  // 解析块
    void parseVarDeclaration(int &ptx, int lev, int &pdx);       // 解析变量声明
    void parseStrDeclaration(int &ptx, int lev, int &pdx);       // 解析字符串声明
    void parseStatement(SymSet &fsys, int &ptx, int lev);        // 解析语句
    void parseWhileStatement(SymSet &fsys, int &ptx, int lev);   // 解析while语句
    void parseIfStatement(SymSet &fsys, int &ptx, int lev);      // 解析if语句
    void parseExpression(SymSet &fsys, int &ptx, int lev);       // 解析表达式
    void parseStringExpression(SymSet &fsys, int &ptx, int lev); // 解析字符串表达式
    void parseArithExpression(SymSet &fsys, int &ptx, int lev);  // 解析算术表达式
    void parseOutputStatement(SymSet &fsys, int &ptx, int lev);  // 解析输出语句
    void parseInputStatement(SymSet &fsys, int &ptx, int lev);   // 解析输入语句
    void parseFunctionCall(SymSet &fsys, int &ptx, int lev);     // 解析函数调用
    void parseCondition(SymSet &fsys, int &ptx, int lev);        // 解析条件
    void parseFactor(SymSet &fsys, int &ptx, int lev);           // 解析因子
    void parseTerm(SymSet &fsys, int &ptx, int lev);             // 解析项
    void parseAssignment(SymSet &fsys, int &ptx, int lev);       // 解析赋值语句
};