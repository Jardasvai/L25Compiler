#include "table.hpp"

// 错误报告函数实现
void reportError(int errCode)
{
    cerr << "Error " << errCode << ": ";
    switch (errCode)
    {
    case 101:
        cerr << "Symbol table overflow" << endl;
        break;
    default:
        cerr << "Unknown error" << endl;
        break;
    }
}

Table::Table(Scanner &scanner):lex(scanner)
{
    table.resize(txmax_val);
}

//返回符号表中第i个元素的引用
Table::Item &Table::get(int i)
{
    if (i < 0 || i >= txmax_val)
        throw std::out_of_range("Table index out of range");
    return table[i];
}

// 添加符号到符号表
// k: 符号类型：constant, string, variable, function
void Table::enter(Objekt k, int lev, int dx, const std::string &name, const std::string &str_value)
{
    if (tx++ >= txmax_val){
        reportError(101);
        return;
    }

    Item &item = get(tx);
    item.name = name.empty() ?lex.id : name;
    item.kind = k;

    switch (k)
    {
    case Objekt::string:
        {
            string value = str_value.empty() ? lex.str_val : str_value;
            if (value.length() > smax_val) reportError(102);
            else item.str_val = value;
            item.level = lev;
            item.adr = dx;
        }
        break;
    case Objekt::int_pointer:
    case Objekt::string_pointer:
    case Objekt::variable:
        item.level = lev;
        item.adr = dx;
        break;

    case Objekt::function:
        item.level = lev;
        item.adr = dx;
        // 过程地址和大小稍后设置
        break;
    }
}

void Table::debugTable(int start)
{
    if (!tableswitch)
        return;

    std::cout << "TABLE:\n";
    fas << "TABLE:\n";
    ftable << "TABLE:\n";

    if (start >= tx)
    {
        std::cout << "Empty table\n";
        fas << "Empty table\n";
        ftable << "Empty table\n";
        return;
    }

    for (int i = start; i < tx; i++)
    {
        Item &item = get(i);

        switch (item.kind)
        {
        case Objekt::string:
            std::cout << i << " " << item.name << " string level:" << item.level << " addr:" << item.adr << " value:\"" << item.str_val << "\"\n";
            fas << i << " " << item.name << " string level:" << item.level << " addr:" << item.adr << " value:\"" << item.str_val << "\"\n";
            ftable << i << " " << item.name << " string level:" << item.level << " addr:" << item.adr << " value:\"" << item.str_val << "\"\n";
            break;


        case Objekt::variable:
            std::cout << i << " " << item.name << " variable level:" << item.level << " addr:" << item.adr << "\n";
            fas << i << " " << item.name << " variable level:" << item.level << " addr:" << item.adr << "\n";
            ftable << i << " " << item.name << " variable level:" << item.level << " addr:" << item.adr << "\n";
            break;

        case Objekt::int_pointer:
            std::cout << i << " " << item.name << " int_pointer level:" << item.level << " addr:" << item.adr << "\n";
            fas << i << " " << item.name << " int_pointer level:" << item.level << " addr:" << item.adr << "\n";
            ftable << i << " " << item.name << " int_pointer level:" << item.level << " addr:" << item.adr << "\n";
            break;

        case Objekt::string_pointer:
            std::cout << i << " " << item.name << " string_pointer level:" << item.level << " addr:" << item.adr << "\n";
            fas << i << " " << item.name << " string_pointer level:" << item.level << " addr:" << item.adr << "\n";
            ftable << i << " " << item.name << " string_pointer level:" << item.level << " addr:" << item.adr << "\n";
            break;
            
        case Objekt::function:
            std::cout << i << " " << item.name << " function level:" << item.level << " size:" << item.size << "\n";
            fas << i << " " << item.name << " function level:" << item.level << " size:" << item.size << "\n";
            ftable << i << " " << item.name << " function level:" << item.level << " size:" << item.size << "\n";
            break;
        }
    }

    std::cout << "\n";
    fas << "\n";
    ftable << "\n";
}

/**
 * 在名字表中查找某个名字的位置
 * 
 * @param idt 要查找的名字
 * @return 如果找到则返回名字项的下标，否则返回-1
 */
int Table::position(const std::string &idt)
{
    // 从后向前搜索，实现最近嵌套原则
    for (int i = tx; i > 0; i--)
    {
        if (table[i].name == idt) return i;
    }
    return -1;
}

Objekt Table::lookup(const std::string &idt)
{
    //  查找符号表中标识符的类型
    int pos = position(idt);
    return pos>=0?table[pos].kind:static_cast<Objekt>(-1);
    // 如果未找到，返回一个特殊值表示未声明的标识符
    // 不应该默认返回variable，这会导致解析器错误地处理未声明的函数
    // 返回无效值，让调用者处理错误
}
