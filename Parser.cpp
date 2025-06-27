#include "Parser.hpp"
#include "Scanner.hpp"
#include "table.hpp"
#include "Interpreter.hpp"
#include "Compiler.hpp"
#include <cstdlib>
#include <vector>

using namespace std;

// 定义静态错误计数器
int Parser::errorCount = 0;

Parser::Parser(Scanner &scanner, Table &symbolTable, Interpreter &interpreter)
    : lex(scanner), table(symbolTable), interp(interpreter), dx(0)
{
    // 设置声明开始符号集
    declbegsys = SymSet(static_cast<size_t>(Symbol::COUNT));
    statbegsys = SymSet(static_cast<size_t>(Symbol::COUNT));
    facbegsys = SymSet(static_cast<size_t>(Symbol::COUNT));

    // 初始化声明开始符号集
    declbegsys.set(Symbol::strsym);
    declbegsys.set(Symbol::funcsym);
    declbegsys.set(Symbol::letsym);
    // declbegsys.set(Symbol::mainsym); // 初始化语句开始符号集
    statbegsys.set(Symbol::ident);
    statbegsys.set(Symbol::ifsym);
    statbegsys.set(Symbol::whilesym);
    statbegsys.set(Symbol::inputsym);
    statbegsys.set(Symbol::outputsym);
    statbegsys.set(Symbol::atsym);

    // 初始化因子开始符号集
    facbegsys.set(Symbol::ident);
    facbegsys.set(Symbol::number);
    facbegsys.set(Symbol::lparen);
    // facbegsys.set(Symbol::string_literal);
    facbegsys.set(Symbol::minus);
}

Parser::~Parser()
{
    // 析构函数
}
void Parser::nextSym()
{
    lex.getsym();
    sym = lex.sym;
}
void Parser::parse()
{
    SymSet nxtlev = SymSet(sym_num);
    // 初始化符号集
    nextSym();

    // 解析 program 关键字
    if (sym == Symbol::programsym){
        nextSym();
        if (sym == Symbol::ident){
            nextSym();
            if (sym == Symbol::lbrace){
                nextSym();
                parseProgram(0,nxtlev);
                if (sym == Symbol::rbrace)
                    cout << "Program parsed successfully!" << endl;
                else
                    error(13); // 期望 '}'
            }
            else{
                error(17); // 期望 '{'
                test(declbegsys, nxtlev, 17);
            }
        }
        else{
            error(4);
            SymSet recovery_set(sym_num);
            recovery_set.set(Symbol::lbrace);
            test(recovery_set, nxtlev, 4);
        }
    }
    else{
        error(18); // 期望 'program'关键字
        SymSet recovery_set(sym_num);
        recovery_set.set(Symbol::programsym);
        recovery_set.set(Symbol::ident);
        test(recovery_set, nxtlev, 18);
    }
        
}

void Parser::test(SymSet &s1, SymSet &s2, int n)
{
    if (!s1.get(sym))
    {
        error(n); // 如果不在s1中，则报错
        while (!s1.get(sym) && !s2.get(sym))
        { // 既不在s1中，也不在s2中，则继续读入下一个符号，尝试继续解析
            nextSym();
        }
    }
}

void Parser::error(int n)
{
    errorCount++;

    // 创建指向错误位置的空格字符串
    string spaces(lex.char_position - 1, ' ');

    // 输出错误指示到控制台
    cout << "**" << spaces << "^" << n << endl;

    // 输出错误指示到文 ?
    if (fas.is_open())
    {
        fas << "**" << spaces << "^" << n << endl;
    }

    if (errorCount > maxerr)
    {
        cerr << "Too many errors, aborting compilation." << endl;
        exit(1);
    }
}

void Parser::parseProgram(int lev,SymSet &fsys)
{
    int tx0, dx0, cx0;                   // 保留初始的tx, dx, cx
    tx0 = table.tx;                      // 保存函数的开始位置
    //table.get(table.tx).adr = interp.cx; // 设置程序入口地址
    int jmp_to_main_addr = interp.cx;
    dx0 = dx;                            // 函数的参数个数
    dx = 3;
    interp.gen(JMP, 0, 0);
    
    if(lev>levmax)error(1);
    // 解析函数定义部分
    while (sym == Symbol::funcsym)
    {
        nextSym(); // 跳过funcsym
        parseFunctionDef(fsys);
    }    // 解析主函数
    if (sym == Symbol::mainsym)
    {
        nextSym();        
        if (sym == Symbol::lbrace){
            nextSym();
            // 为main函数分配变量空间
            int mainStartAddr = interp.cx;
            interp.setCode(jmp_to_main_addr, JMP, 0, mainStartAddr);

            int int_addr = interp.cx; // 保存INT指令位置
            interp.gen(INT, 0, 0);  // 为main函数分配空间
            dx = 3;
            // 回填main函数的入口地址
            parseStatementList(fsys, 1);  // main函数也使用level=1
            interp.setCode(int_addr, INT, 0, dx); // 回填INT指令的地址

            if (sym == Symbol::rbrace)
            {
                nextSym();
                // 程序结束，生成HLT指令
                interp.gen(HLT, 0, 0);
            }
            else
                error(13); // 期望 '}'
        }
        else{
            error(17); // 期望 '{'
            test(statbegsys, fsys, 17);
        }
            
    }
    else{
        error(19); // 期望 'main'
        SymSet recovery_set(sym_num);
        recovery_set.set(Symbol::mainsym);
        recovery_set.set(Symbol::lbrace);
        test(recovery_set, fsys, 19);
    }
        
}

void Parser::parseFunctionDef(SymSet &fsys)
{
    if (sym == Symbol::ident)
    {
        string funcName = lex.id;
        table.enter(Objekt::function, 0, 0, funcName);
        int funcIndex = table.tx;
        //int jmpAddr = interp.cx;
        //interp.gen(JMP, 0, 0);  // 生成跳转指令跳过函数体
        nextSym();

        if (sym == Symbol::lparen)
        {
            nextSym();
            // 保存当前的dx值，用于函数局部变量分配
            int dx0 = dx;
            dx = 3;  // 为函数的返回地址、静态链、动态链预留空间
            
            if (sym != Symbol::rparen) parseParameterList(fsys);
            if (sym == Symbol::rparen)
            {
                nextSym();  //跳过') '
                if (sym == Symbol::lbrace)
                {   
                    nextSym();      //跳过'{'
                    int funcStartAddr = interp.cx;            // 保存函数真正的入口地址
                    int int_addr = interp.cx; // 保存INT指令位置
                    interp.gen(INT, 0, 0);                   // 生成空间分配指令
                    table.get(funcIndex).adr = funcStartAddr; // 立即设置函数入口地址为INT指令位置
                    parseStatementList(fsys, 1);  // 函数内部使用level=1
                    if (sym != Symbol::returnsym) error(20); // 期望return
                    else
                    {
                        nextSym(); // 跳过 'return'
                        // 解析返回表达式
                        SymSet set1 = fsys;
                        set1.set(Symbol::semicolon);
                        parseExpression(set1, table.tx, 1); // 使用正确的level=1
                        interp.gen(OPR, 0, 0);
                        if (sym == Symbol::semicolon)
                            nextSym();
                        else
                            error(5); // 期望分号
                    }
                    interp.setCode(int_addr, INT, 0, dx); // 回填INT指令的地址
                    //interp.setCode(jmpAddr, JMP, 0, interp.cx);  // 回填跳转地址
                    dx = dx0;  // 恢复dx值

                    if (sym == Symbol::rbrace)
                        nextSym();
                    else
                        error(13); // 期望 '}'
                }
                else{
                    error(17); // 期望 '{'
                    test(statbegsys, fsys, 17);
                }   
            }
            else
                error(22); // 期望 ')'
        }
        else{
            error(21); // 期望 '('
            SymSet recovery_set = SymSet(sym_num);
            recovery_set.set(Symbol::ident);
            recovery_set.set(Symbol::rparen);
            test(recovery_set, fsys, 21);
        }
            
    }
    else{
        error(4); // 期望标识符
        test(facbegsys, fsys, 4); // 尝试恢复到能解析下一个符号的地方
        return; 
    }
        
}

void Parser::parseParameterList(SymSet &fsys)
{
    // 参数列表：ident (',' ident)*
    vector<string> paramNames; // 临时存储参数名
    
    do
    {
        if (sym == Symbol::ident)
        {
            paramNames.push_back(lex.id);
            nextSym();
        }
        else error(4); // 期望标识符

        if (sym == Symbol::comma) nextSym(); // 跳过逗号
        else break;
    } while (true);
    
    // 现在我们知道了参数总数，可以正确分配地址
    int paramCount = paramNames.size();
    for (int i = 0; i < paramCount; i++)
    {
        // 第一个参数在 -paramCount，第二个在 -(paramCount-1)，...，最后一个在 -1
        // 这样与栈布局一致：b-paramCount = 第一个参数，b-1 = 最后一个参数
        int addr = -(paramCount - i+1);
        table.enter(Objekt::variable, 1, addr, paramNames[i]);
    }
}

void Parser::parseStatement(SymSet &fsys, int &ptx, int lev)
{
    switch (sym)
    {
    case Symbol::atsym:
    case Symbol::ident:
        // 检查是否是函数调用
        if (sym == Symbol::ident && table.position(lex.id) >= 0 && table.lookup(lex.id) == Objekt::function)
        {
            parseFunctionCall(fsys, ptx, lev);
        }
        else
        {
            parseAssignment(fsys, ptx, lev);
        }
        break;
    case Symbol::letsym:
        parseVarDeclaration(ptx, lev, dx);
        break;
    case Symbol::ifsym:
        parseIfStatement(fsys, ptx, lev);
        break;
    case Symbol::whilesym:
        parseWhileStatement(fsys, ptx, lev);
        break;
    case Symbol::inputsym:
        parseInputStatement(fsys, ptx, lev);
        break;
    case Symbol::outputsym:
        parseOutputStatement(fsys, ptx, lev);
        break;
    case Symbol::strsym:
        parseStrDeclaration(ptx, lev, dx);
        break;
    default:
        error(23); // 非法语句
        nextSym(); // 跳过错误符号
        break;
    }
}

void Parser::parseWhileStatement(SymSet &fsys, int &ptx, int lev)
{
    nextSym(); // 跳过 'while'

    // 必须包含括号
    if (sym != Symbol::lparen)
    {
        error(21); // 期望 '('
        test(facbegsys, fsys, 21);
        //return;
    }
    nextSym(); // 跳过 '('

    int loopaddr = interp.cx; // 保存循环开始地址

    SymSet set1 = SymSet(static_cast<size_t>(Symbol::COUNT));
    set1 = fsys;
    set1.set(Symbol::rparen);

    parseCondition(set1, ptx, lev);

    if (sym != Symbol::rparen) error(22); // 期望 ')'
    else nextSym(); // 跳过 ')'

    int jmpaddr = interp.cx; // 保存条件跳转地址
    interp.gen(JPC, 0, 0);   // 条件跳转，地址待填

    // 必须有大括号
    if (sym != Symbol::lbrace)
    {
        error(17); // 期望 '{'
        // return;
        test(statbegsys, fsys, 17);
    }else{
        nextSym(); // 跳过 '{'
    }

    parseStatementList(fsys,lev);

    if (sym != Symbol::rbrace) error(13); // 期望 '}'
    else nextSym(); // 跳过 '}'

    interp.gen(JMP, 0, loopaddr);               // 跳回循环开始
    interp.setCode(jmpaddr, JPC, 0, interp.cx); // 回填条件跳转地址
}

void Parser::parseIfStatement(SymSet &fsys, int &ptx, int lev)
{
    nextSym(); // 跳过 'if'

    // 必须包含括号
    if (sym != Symbol::lparen)
    {
        error(21); // 期望 '('
        SymSet recovery_set = facbegsys;
        test(recovery_set, fsys, 21);
    }
    nextSym(); // 跳过 '('

    SymSet set1 = SymSet(static_cast<size_t>(Symbol::COUNT));
    set1 = fsys;
    set1.set(Symbol::rparen);

    parseCondition(set1, ptx, lev);

    if (sym != Symbol::rparen) error(22); // 期望 ')'
    else nextSym(); // 跳过 ')'

    int jmpaddr = interp.cx; // 保存条件跳转地址
    interp.gen(JPC, 0, 0);   // 条件跳转，地址待填

    // 必须有大括号
    if (sym != Symbol::lbrace)
    {
        error(17); // 期望 '{'
        SymSet recovery_set(sym_num);
        recovery_set.set(Symbol::lbrace);
        test(recovery_set, fsys, 22);
    }
    nextSym(); // 跳过 '{'

    parseStatementList(fsys,lev);

    if (sym != Symbol::rbrace) error(13); // 期望 '}'
    else nextSym(); // 跳过 '}'

    int jmpaddr2 = 0;
    if (sym == Symbol::elsesym)
    {
        jmpaddr2 = interp.cx;  // 保存无条件跳转地址
        interp.gen(JMP, 0, 0); // 无条件跳转，地址待填

        interp.setCode(jmpaddr, JPC, 0, interp.cx); // 回填条件跳转地址

        nextSym(); // 跳过 'else'

        // else部分也必须有大括号
        if (sym != Symbol::lbrace)
        {
            error(17); // 期望 '{'
            test(statbegsys, fsys, 17);
        }else{
            nextSym(); // 跳过 '{'
        }

        parseStatementList(fsys,lev);

        if (sym != Symbol::rbrace) error(13); // 期望 '}'
        else nextSym(); // 跳过 '}'

        interp.setCode(jmpaddr2, JMP, 0, interp.cx); // 回填无条件跳转地址
    }
    else interp.setCode(jmpaddr, JPC, 0, interp.cx); // 回填条件跳转地址
}

void Parser::parseOutputStatement(SymSet &fsys, int &ptx, int lev)
{
    nextSym(); // 跳过 'output'

    // 必须有括号
    if (sym != Symbol::lparen)
    {
        error(21); // 期望 '('
        test(facbegsys, fsys, 21);
    }
    nextSym(); // 跳过 '('

    // 解析参数列表
    do
    {
        SymSet set1 = SymSet(static_cast<size_t>(Symbol::COUNT));
        set1 = fsys;
        set1.set(Symbol::comma);
        set1.set(Symbol::rparen);
        parseExpression(set1, ptx, lev);
        interp.gen(WRT, 0, 0); // 生成输出指令

        if (sym == Symbol::comma) nextSym(); // 跳过逗号
        else break;
    } while (true);

    if (sym != Symbol::rparen) error(22); // 期望 ')'
    else nextSym(); // 跳过 ')'
}

void Parser::parseInputStatement(SymSet &fsys, int &ptx, int lev)
{
    nextSym(); // 跳过 'input'

    // 必须有括号
    if (sym != Symbol::lparen)
    {
        error(21); // 期望 '('
        test(facbegsys, fsys, 21);
    }
    nextSym(); // 跳过 '('

    // 解析参数列表
    do
    {
        if (sym == Symbol::ident)
        {
            int i = table.position(lex.id);
            if (i >= 0)
            {
                Table::Item item = table.get(i);
                if (item.kind == Objekt::variable)
                {
                    interp.gen(RED, 0, 0); // 读操作
                    interp.gen(STO, lev - item.level, item.adr);
                }
                else error(12); // 不能对该标识符赋值
            }
            else error(11); // 未声明的标识符
            nextSym();
        }
        else error(4); // 期望标识符

        if (sym == Symbol::comma) nextSym(); // 跳过逗号
        else break;
    } while (true);

    if (sym != Symbol::rparen) error(22); // 期望 ')'
    else nextSym(); // 跳过 ')'
}

void Parser::parseStatementList(SymSet &fsys, int lev)
{
    // 根据L25 EBNF: stmt_list ::= ( stmt ';' )+
    bool hasStatement = false;

    while (statbegsys.get(sym)||(declbegsys.get(sym)&&sym!=Symbol::funcsym))
    {
        hasStatement = true; // 标记已经解析到语句
        this->argsToClean = 0; // 重置参数清理计数器
        parseStatement(fsys, table.tx, lev);
        //if(this->argsToClean > 0)interp.gen(INT, 0, -this->argsToClean); // 清理参数
        if (sym == Symbol::semicolon) nextSym();
        else
        {
            error(5); // 期望分号

            // 错误恢复：跳过当前符号，直到找到分号或语句开始符号
            // while (sym != Symbol::semicolon &&
            //        !statbegsys.get(sym) &&
            //        sym != Symbol::rbrace &&
            //        sym != Symbol::nul) nextSym();

            // // 如果找到分号，跳过它
            // if (sym == Symbol::semicolon) nextSym();
            SymSet recovery_set = statbegsys; // 下一个语句的开始
            recovery_set.set(Symbol::rbrace);   // 或者 '}'
            test(recovery_set, fsys, 5); 
        }
    }

    // 如果没有解析到任何语句，报错
    if (!hasStatement) error(23); // 期望语句
}

void Parser::parseVarDeclaration(int &ptx, int lev, int &pdx)
{
    nextSym(); // 跳过 'let'
    bool is_pointer = false;
    if (sym == Symbol::atsym){
        is_pointer = true; // 解析指针类型
        nextSym(); // 跳过 '@'
    }
    if (sym == Symbol::ident)
    {
        string varName = lex.id;
        // 统一使用variable类型
        table.enter(is_pointer? Objekt::int_pointer:Objekt::variable, lev, pdx, varName);
        int varAddr = pdx;
        pdx++;
        nextSym(); // 检查是否有初始化赋值

        if (sym == Symbol::becomes)
        {
            nextSym(); // 跳过 '='
            if (sym == Symbol::addresssym) {
                if (!is_pointer) error(28); // 错误：不能把地址赋给非指针变量
                nextSym(); // 跳过 '&'
                if (sym == Symbol::ident) {
                    int i = table.position(lex.id);
                    if (i >= 0) {
                        Table::Item item = table.get(i);
                        // 生成 LDA 指令加载地址
                        interp.gen(LDA, lev - item.level, item.adr);
                        nextSym();
                    } else {
                        error(11); // 未声明的标识符
                    }
                } else {
                    error(4); // '&' 后面需要一个标识符
                    }
                }else{
                    SymSet set1 = SymSet(static_cast<size_t>(Symbol::COUNT));
                    set1.set(Symbol::semicolon);
                    parseExpression(set1, ptx, lev);
            }
            interp.gen(STO, 0, varAddr);  // 使用0作为level差值，因为在同一级
        }
    }
    else error(4); // 期望标识符
}
void Parser::parseStrDeclaration(int &ptx, int lev, int &pdx)
{
    nextSym(); // 跳过 'str'
    bool is_pointer = false;
    if (sym == Symbol::atsym){
        is_pointer = true; // 解析指针类型
        nextSym(); // 跳过 '@'
    }
    if (sym == Symbol::ident)
    {
        string strName = lex.id;
        table.enter(is_pointer?Objekt::string_pointer:Objekt::string, lev, pdx, strName);
        int addr = pdx; // store variable address
        pdx++;
        nextSym(); // 支持字符串初始化，如 str s = "hello";
        if (sym == Symbol::becomes)
        {
            nextSym(); // 跳过 '='
            if (sym == Symbol::addresssym) {
                if (!is_pointer) error(28); // 错误：不能把地址赋给非指针变量
                nextSym(); // 跳过 '&'
                if (sym == Symbol::ident) {
                    int i = table.position(lex.id);
                    if (i >= 0) {
                        Table::Item item = table.get(i);
                        // 生成 LDA 指令加载地址
                        interp.gen(LDA, lev - item.level, item.adr);
                        nextSym();
                    } else {
                        error(11); // 未声明的标识符
                    }
                } else {
                    error(4); // '&' 后面需要一个标识符
                }
            } else {
                SymSet set1 = SymSet(static_cast<size_t>(Symbol::COUNT));
                set1.set(Symbol::semicolon);
                parseExpression(set1, ptx, lev);
            }
            interp.gen(STO, 0, addr);  // 使用0作为level差值
        }
        // 不在这里处理分号，由parseStatementList处理
    }
    else error(4); // 期望标识符
}

void Parser::parseFunctionCall(SymSet &fsys, int &ptx, int lev)
{
    string funcName = lex.id;
    int i = table.position(funcName);
    if (i >= 0)
    {
        Table::Item item = table.get(i);
        if (item.kind == Objekt::function)
        {
            nextSym();
            if (sym == Symbol::lparen)
            {
                nextSym();
                int arg_count = 0; // 记录参数个数
                // 解析参数列表: arg_list ::= expr ( ',' expr )*
                if (sym != Symbol::rparen)
                {
                    // 为每个表达式创建简单的符号集合
                    SymSet exprSet = SymSet(static_cast<size_t>(Symbol::COUNT));
                    exprSet.set(Symbol::comma);
                    exprSet.set(Symbol::rparen);

                    parseExpression(exprSet, ptx, lev);
                    arg_count++;    //解析到第一个参数，计数器加一
                    while (sym == Symbol::comma)
                    {
                        nextSym();
                        parseExpression(exprSet, ptx, lev);
                        arg_count++;    //每解析一个参数，计数器加一
                    }
                }

                if (sym == Symbol::rparen)
                {
                    nextSym();
                    interp.gen(CAL, lev - item.level, item.adr);
                    if(arg_count>0)this->argsToClean+=arg_count;//interp.gen(INT, 0, -arg_count);
                    // 使用负数参数的INT指令来减少栈顶指针，实现弹出参数
                }
                else error(22); // 期望 ')'
            }
            else error(21); // 期望 '('
        }
        else error(25); // 不是函数
    }
    else error(11); // 未声明的标识符
}
void Parser::parseExpression(SymSet &fsys, int &ptx, int lev)
{ // 根据EBNF: expr ::= str_expr | arith_expr
    // 首先检查是否是字符串表达式
    if (sym == Symbol::string_literal ||
        (sym == Symbol::ident && table.position(lex.id) >= 0 && table.lookup(lex.id) == Objekt::string))
        parseStringExpression(fsys, ptx, lev);
    else parseArithExpression(fsys, ptx, lev);
}

void Parser::parseStringExpression(SymSet &fsys, int &ptx, int lev)
{
    // str_expr ::= str ( '*' ( number | arith_expr ) | '+' ( number | str | arith_expr ) )    // 解析第一个字符串
    if (sym == Symbol::string_literal)
    {
        interp.genString(lex.str_val);
        nextSym();
    }
    else if (sym == Symbol::ident)
    {
        int i = table.position(lex.id);
        if (i >= 0)
        {
            Table::Item item = table.get(i);
            if (item.kind == Objekt::string)
            {
                interp.gen(LOD, lev - item.level, item.adr);
                nextSym();
            }
            else
            {
                error(26); // 期望字符串
                return;
            }
        }
        else
        {
            error(11); // 未声明的标识符
            return;
        }
    }

    // 检查是否有字符串运算
    while (sym == Symbol::times || sym == Symbol::plus)
    {
        Symbol op = sym;
        nextSym();

        if (op == Symbol::times)
        {
            // 字符串乘法：str * (number | arith_expr)
            if (sym == Symbol::number)
            {
                interp.gen(LIT, 0, lex.num);
                nextSym();
                interp.gen(OPR, 0, 17); // 字符串重复操作
            }
            else
            {
                parseArithExpression(fsys, ptx, lev);
                interp.gen(OPR, 0, 17); // 字符串重复操作
            }
        }
        else if (op == Symbol::plus)
        {
            // 字符串加法：str + (number | str | arith_expr)
            if (sym == Symbol::string_literal)
            {
                interp.genString(lex.str_val);
                nextSym();
                interp.gen(OPR, 0, 18); // 字符串连接操作
            }
            else if (sym == Symbol::ident && table.position(lex.id) >= 0 && table.lookup(lex.id) == Objekt::string)
            {
                int i = table.position(lex.id);
                Table::Item item = table.get(i);
                interp.gen(LOD, lev - item.level, item.adr);
                nextSym();
                interp.gen(OPR, 0, 18); // 字符串连接操作
            }
            else if (sym == Symbol::number)
            {
                interp.gen(LIT, 0, lex.num);
                nextSym();
                interp.gen(OPR, 0, 19); // 字符串与数字连接操作
            }
            else if (sym == Symbol::ident && table.position(lex.id) >= 0 && table.lookup(lex.id) == Objekt::variable)
            {
                // 处理字符串与变量的连接（变量包含数字）
                int i = table.position(lex.id);
                Table::Item item = table.get(i);
                interp.gen(LOD, lev - item.level, item.adr);
                nextSym();
                interp.gen(OPR, 0, 19); // 字符串与数字连接操作
            }
            else
            {
                parseArithExpression(fsys, ptx, lev);
                interp.gen(OPR, 0, 19); // 字符串与数字连接操作
            }
        }
    }
}

void Parser::parseArithExpression(SymSet &fsys, int &ptx, int lev)
{
    Symbol addop;

    if (sym == Symbol::plus || sym == Symbol::minus)
    {
        addop = sym;
        nextSym();
        parseTerm(fsys, ptx, lev);
        if (addop == Symbol::minus) interp.gen(OPR, 0, 1); // 取负
    }
    else parseTerm(fsys, ptx, lev);

    while (sym == Symbol::plus || sym == Symbol::minus)
    {
        addop = sym;
        nextSym();
        parseTerm(fsys, ptx, lev);
        if (addop == Symbol::plus) interp.gen(OPR, 0, 2); // 加法
        else interp.gen(OPR, 0, 3); // 减法
    }
}
void Parser::parseCondition(SymSet &fsys, int &ptx, int lev)
{
    Symbol relop;
    SymSet set1 = SymSet(static_cast<size_t>(Symbol::COUNT));

    // L25不支持odd操作符，直接解析布尔表达式
    set1 = fsys;
    set1.set(Symbol::eql);
    set1.set(Symbol::neq);
    set1.set(Symbol::lss);
    set1.set(Symbol::leq);
    set1.set(Symbol::gtr);
    set1.set(Symbol::geq);

    parseExpression(set1, ptx, lev);

    if (sym == Symbol::eql || sym == Symbol::neq || sym == Symbol::lss ||
        sym == Symbol::leq || sym == Symbol::gtr || sym == Symbol::geq)
    {
        relop = sym;
        nextSym();
        parseExpression(fsys, ptx, lev);

        switch (relop)
        {
        case Symbol::eql:
            interp.gen(OPR, 0, 8);
            break;
        case Symbol::neq:
            interp.gen(OPR, 0, 9);
            break;
        case Symbol::lss:
            interp.gen(OPR, 0, 10);
            break;
        case Symbol::geq:
            interp.gen(OPR, 0, 11);
            break;
        case Symbol::gtr:
            interp.gen(OPR, 0, 12);
            break;
        case Symbol::leq:
            interp.gen(OPR, 0, 13);
            break;
        default:
            break;
        }
    }
    else{
        error(16); // 期望关系操作符
        test(facbegsys, fsys, 16);
    } 
}
void Parser::parseAssignment(SymSet &fsys, int &ptx, int lev)
{
    if (sym == Symbol::atsym) { // 赋值给 @p
        nextSym(); // 跳过 '@'
        if (sym == Symbol::ident) {
             int i = table.position(lex.id);
             if (i >= 0) {
                Table::Item item = table.get(i);
                // 检查变量是否为指针类型
                if (item.kind != Objekt::int_pointer && item.kind != Objekt::string_pointer) {
                    error(29); // 错误：试图解引用一个非指针变量
                }
                // 加载指针p的值(它是一个地址)到栈顶
                interp.gen(LOD, lev - item.level, item.adr);
                nextSym();
                if (sym == Symbol::becomes) {
                    nextSym();
                    parseExpression(fsys, ptx, lev); // 解析右侧表达式
                    interp.gen(STI, 0, 0); // 生成间接存储指令
                } else {
                    error(27); // 期望 '='
                    test(facbegsys, fsys, 27);
                }
             } else {
                error(11); // 未声明的标识符
             }
        } else {
            error(4); // '@' 后面需要一个标识符
        }
    }else if(sym == Symbol::ident){
        string varName = lex.id;
        int i = table.position(varName);
        if (i >= 0)
        {
            Table::Item item = table.get(i);
            if (item.kind == Objekt::variable || item.kind == Objekt::string|| item.kind == Objekt::int_pointer || item.kind == Objekt::string_pointer)
            {
                nextSym();
                if (sym == Symbol::becomes)
                {
                    nextSym();
                    parseExpression(fsys, ptx, lev);
                    interp.gen(STO, lev - item.level, item.adr);
                }
                else error(27); // 期望 '='
            }
            else error(12); // 不能对该标识符赋值
        }
        else error(11); // 未声明的标识符
    }
}
void Parser::parseTerm(SymSet &fsys, int &ptx, int lev)
{
    Symbol mulop;

    // 解析第一个因子，不在乘除符号处停止
    parseFactor(fsys, ptx, lev);

    while (sym == Symbol::times || sym == Symbol::divide)
    {
        mulop = sym;
        nextSym();
        // 解析后续因子，不在乘除符号处停止
        parseFactor(fsys, ptx, lev);
        if (mulop == Symbol::times) interp.gen(OPR, 0, 4); // 乘法
        else interp.gen(OPR, 0, 5); // 除法
    }
}
void Parser::parseFactor(SymSet &fsys, int &ptx, int lev)
{
    if (sym == Symbol::atsym) { // 新增：处理解引用 @p
        nextSym();
        parseFactor(fsys, ptx, lev); // 递归解析因子，结果应为一个地址
        interp.gen(LDI, 0, 0);       // 生成 LDI 指令，加载该地址处的值
    }
    else if (sym == Symbol::addresssym) { // 新增：处理取地址 &v
        nextSym();
        if (sym == Symbol::ident) {
            int i = table.position(lex.id);
            if (i >= 0) {
                Table::Item item = table.get(i);
                interp.gen(LDA, lev - item.level, item.adr);
                nextSym();
            } else {
                error(11); // 未声明的标识符
            }
        } else {
            error(4); // '&' 后面需要一个标识符
        }
    } 
    else if (sym == Symbol::ident)
    {
        string identName = lex.id;
        int i = table.position(identName);
        nextSym();

        // 先检查是否是函数调用（根据EBNF，通过左括号判断）
        if (sym == Symbol::lparen)
        {
            //这是函数调用
            // int i = table.position(identName);
            if (i >= 0)
            {
                Table::Item item = table.get(i);
                if (item.kind == Objekt::function)
                {
                    nextSym(); // 跳过 '('
                    // 解析参数列表: arg_list ::= expr ( ',' expr )*
                    int arg_count = 0; // 声明参数计数器
                    if (sym != Symbol::rparen)
                    {
                        SymSet exprSet = fsys;
                        exprSet.set(Symbol::comma);
                        exprSet.set(Symbol::rparen);
                        // 简化处理：直接调用parseExpression，不传递复杂的符号集合
                        // SymSet emptySet = SymSet(static_cast<size_t>(Symbol::COUNT));
                        // parseExpression(emptySet, ptx, lev);
                        parseExpression(exprSet, ptx, lev);
                        arg_count++; // 解析到第一个参数，计数器加一
                        while (sym == Symbol::comma)
                        {
                            nextSym();
                            parseExpression(exprSet, ptx, lev);
                            arg_count++; // 每解析一个参数，计数器加一
                        }
                    }
                    if (sym == Symbol::rparen) nextSym();
                    else error(22); // 期望 ')'
                    interp.gen(CAL, lev - item.level, item.adr);
                    if(arg_count>0)this->argsToClean+=arg_count;//interp.gen(INT, 0, -arg_count);
                }
                else error(25); // 不是函数
            }
            else error(11); // 未声明的标识符
        }
        else
        {
            // 这是变量或常量
            int i = table.position(identName);
            if (i >= 0)
            {
                Table::Item item = table.get(i);
                if (item.kind == Objekt::constant) interp.gen(LIT, 0, item.value);
                else if (item.kind == Objekt::variable || item.kind == Objekt::string || item.kind == Objekt::int_pointer || item.kind == Objekt::string_pointer) {
                    interp.gen(LOD, lev - item.level, item.adr);
                }
                // else if (item.kind == Objekt::string) {
                //     interp.gen(LOD, lev - item.level, item.adr);
                // }
                else error(21); // 不能在此处使用该标识符
            }
            else error(11); // 未声明的标识符
        }
    }
    else if (sym == Symbol::number)
    {
        interp.gen(LIT, 0, lex.num);
        nextSym();
    }
    else if (sym == Symbol::string_literal)
    {
        // 字符串字面量处理，生成字符串并推入地址
        interp.genString(lex.str_val);
        nextSym();
    }
    else if (sym == Symbol::lparen)
    {
        nextSym();
        // 括号内的表达式应该完整解析，不受外部符号集合约束
        // 只需要保证遇到右括号时停止
        SymSet exprSet = fsys;       // 继承外层符号集合
        exprSet.set(Symbol::rparen); // 添加右括号作为终止符
        parseExpression(exprSet, ptx, lev);
        if (sym == Symbol::rparen) nextSym();
        else error(22); // 期望 ')'
    }
    else
    {
        // 对于不认识的符号，报错但不恢复
        error(24); // 非法的因子
        nextSym();
    }
}
