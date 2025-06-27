#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "Compiler.hpp"
#include "Scanner.hpp"
using namespace std;

extern bool tableswitch;    // 是否打印符号表
extern ostringstream fas_stream;   // 输出文件流

// 错误报告函数声明
void reportError(int errCode);

// 符号类型枚举
enum class Objekt
{
    constant,       // 常量，应该不会用到
    string,
    variable,
    function,
    int_pointer,
    string_pointer
};

class Table
{
public: // 符号表项结构体
    struct Item
    {
        std::string name; // 名字
        Objekt kind;      // 类型
        string str_val;   // 字符串值 (仅当 kind 为 string 时使用)
        int value = 0;    // 值 (仅当 kind 为 constant 时使用)
        int level = 0;    // 层级
        int adr = 0;      // 地址
        int size = 0;     // 大小 (过程)
    };
    int tx = 0; // 当前符号表项指针
    Scanner &lex;   //对词法分析器的引用
private:
    std::vector<Item> table; // 符号表存储

public:
    /**
     * 构造函数，初始化符号表容量
     */
    Table(Scanner &lex);

    /**
     * 获得符号表某一项的内容
     *
     * @param i 符号表中的位置
     * @return 符号表第 i 项的内容
     */
    Item &get(int i); 
    /**
     * 把符号添加到符号表中
     *
     * @param k   符号类型：constant, variable, procedure
     * @param lev 名字所在的层次
     * @param dx  当前应分配的变量的相对地址
     * @param name 符号名称
     * @param str_value 字符串值（仅用于字符串类型）
     */
    void enter(Objekt k, int lev, int dx, const std::string &name = "", const std::string &str_value = "");

    /**
     * 打印符号表内容
     *
     * @param start 当前作用域符号表区间的左端
     */
    void debugTable(int start); 
    /**
     * 在符号表中查找名字的位置
     *
     * @param idt 要查找的名字
     * @return 如果找到则返回名字项的下标，否则返回-1
     */
    int position(const std::string &idt);

    /**
     * 查找符号的类型
     *
     * @param idt 要查找的名字
     * @return 如果找到则返回符号的类型，否则返回Objekt::variable（默认）
     */
    Objekt lookup(const std::string &idt);
};
