#include "Interpreter.hpp"
#include "Compiler.hpp"
#include <stdexcept>

// Static member definitions for string storage
// int Interpreter::sptr = 0;
// std::unordered_map<int, std::string> Interpreter::stringHeap;

void Interpreter::gen(Fct x, int y, int z)
{
    if (cx >= cxmax)
    {
        throw overflow_error("Code array overflow");
    }
    code[cx++] = Instruction(x, y, z);
}

void Interpreter::listCode(int cx0)
{
    if (listswitch)
    {
        if (cx0 < 0 || cx0 >= cx)
            return;

        for (int i = cx0; i < cx; ++i)
        {
            cout << i << ": ";
            fas << i << ": ";
            fcode << i << ": ";

            string opName;
            switch (code[i].f)
            {
            case LIT:
                cout << "LIT";
                fcode << "LIT";
                fas  << "LIT";
                break;
            case OPR:
                cout << "OPR";
                fcode << "OPR";
                fas  << "OPR";
                break;
            case LOD:
                cout << "LOD";
                fcode << "LOD";
                fas  << "LOD";
                break;
            case STO:
                cout << "STO";
                fcode << "STO";
                fas  << "STO";
                break;
            case CAL:
                cout << "CAL";
                fcode << "CAL";
                fas  << "CAL";
                break;
            case INT:
                cout << "INT";
                fcode << "INT";
                fas  << "INT";
                break;
            case JMP:
                cout << "JMP";
                fcode << "JMP";
                fas  << "JMP";
                break;
            case JPC:
                cout << "JPC";
                fcode << "JPC";
                fas  << "JPC";
                break;
            case WRT:
                cout << "WRT";
                fcode << "WRT";
                fas  << "WRT";
                break;
            case RED:
                cout << "RED";
                fcode << "RED";
                fas  << "RED";
                break;
            case HLT:
                cout << "HLT";
                fcode << "HLT";
                fas  << "HLT";
                break;
            case LDA:
                cout << "LDA";
                fcode << "LDA";
                fas  << "LDA";
                break;
            case LDI:
                cout << "LDI";
                fcode << "LDI";
                fas  << "LDI";
                break;
            case STI:
                cout << "STI";
                fcode << "STI";
                fas  << "STI";
                break;

            }
            cout << " " << code[i].l << " " << code[i].a << endl;
            fcode << " " << code[i].l << " " << code[i].a << endl;
            fas << " " << code[i].l << " " << code[i].a << endl;
        }
    }
}

// void Interpreter::genString(const std::string &s)
// {
//     int addr = STRING_BASE + sptr;
//     stringHeap[addr] = s;
//     // generate literal address
//     gen(LIT, 0, addr);
//     sptr += (int)s.size();
// }
void Interpreter::genString(const std::string &str)
{
    // 编译时字符串存入常量区域
    int addr = sptr_const;
    if (sptr_const + str.length() + 1 > STRING_RUNTIME_BASE)
    {
        throw std::overflow_error("Constant string heap overflow");
    }

    // 将字符复制到整型数组中
    for (char c : str)
    {
        s[sptr_const++] = static_cast<int>(c);
    }
    s[sptr_const++] = 0; // 使用整数0作为空终止符

    // 生成加载字符串地址的指令
    gen(LIT, 0, addr);
}
void Interpreter::interpret()
{
    int p, b, t;
    cout << "Start L25" << endl;
    fresult << "Start L25" << endl;
    fas << "Start L25" << endl;

    // Don't reset string storage - strings are compiled into stringHeap
    // sptr and stringHeap should keep their compile-time values

    p = 0;    // Initialize instruction pointer
    b = 1;    // Initialize base pointer (like PL/0)
    t = 3;    // Initialize stack top pointer
    s[0] = 0; // s[0] not used
    s[1] = 0; // Main program's three link units set to 0
    s[2] = 0;
    s[3] = 0;
    while (p < cx)
    {
        const Instruction &inst = code[p++];
        switch (inst.f)
        {
        case LIT:
            if (t + 1 > STACK_MAX) throw std::overflow_error("Stack overflow");
            t = t + 1;
            s[t] = inst.a;
            break;
        case OPR:
            switch (inst.a)
            {
            case 0:
            {
                // 函数返回：恢复调用者的环境并清理参数
                // 当前栈布局：[...] [参数1] [参数2] ... [参数n] [静态链] [动态链] [返回地址] [局部变量...] [返回值]
                // b指向静态链位置
                int returnValue = s[t]; // 保存返回值
                int caller_t = s[b-1];              // 恢复栈指针到最后一个参数的位置
                p = s[b + 2];           // 恢复返回地址（b+2是返回地址的位置）
                b = s[b + 1];          // 恢复动态链（b+1是动态链的位置）
                t = caller_t;
                s[t] = returnValue; // 将返回值放在参数的位置上
                break;
            }
            case 1:
                s[t - 1] = -s[t - 1];
                break;
            case 2:
                t = t - 1;
                s[t] = s[t] + s[t + 1];
                break;
            case 3:
                t = t - 1;
                s[t] = s[t] - s[t + 1];
                break;
            case 4:
                t = t - 1;
                s[t] = s[t] * s[t + 1];
                break;
            case 5:
                t = t - 1;
                if (s[t + 1] == 0)
                {
                    throw runtime_error("Division by zero");
                }
                s[t] = s[t] / s[t + 1];
                break;
            case 6:
                s[t] = s[t] % 2;
                break;
            case 8:
                t = t - 1;
                s[t] = (s[t] == s[t + 1]) ? 1 : 0;
                break;
            case 9:
                t = t - 1;
                s[t] = (s[t] != s[t + 1]) ? 1 : 0;
                break;
            case 10:
                t = t - 1;
                s[t] = (s[t] < s[t + 1]) ? 1 : 0;
                break;
            case 11:
                t = t - 1;
                s[t] = (s[t] >= s[t + 1]) ? 1 : 0;
                break;
            case 12:
                t = t - 1;
                s[t] = (s[t] > s[t + 1]) ? 1 : 0;
                break;
            case 13:
                t = t - 1;
                s[t] = (s[t] <= s[t + 1]) ? 1 : 0;
                break;
            case 14:
            {
                int v = s[t];
                t = t - 1;
                // 检查该值是否是我们字符串堆中的地址
                if (v >= STRING_CONSTANT_BASE && v <= STRING_HEAP_MAX)
                {
                    int addr = v;
                    while (s[addr] != 0)
                    {
                        cout << static_cast<char>(s[addr]);
                        fresult << static_cast<char>(s[addr]);
                        fas << static_cast<char>(s[addr]);
                        addr++;
                    }
                }
                else
                {
                    cout << v; // 它是一个数字
                    fresult << v; // 它是一个数字
                    fas << v;
                }
                cout << endl;
                fresult << endl;
                fas << endl;
                break;
            }
            break;
            case 15:
                cout << "\n";
                fresult << "\n";    
                fas << "\n";
                break;
            case 16:
                if (t + 1 > STACK_MAX) throw std::overflow_error("Stack overflow");
                t = t + 1;
                cin >> s[t];
                fas << s[t] << endl;
                fresult << s[t] << endl; // 记录输入的值
                break;
            case 17:
            {
                int num = s[t];
                int strAddr = s[t - 1];
                t = t - 1;

                if (strAddr < STRING_CONSTANT_BASE || strAddr > STRING_HEAP_MAX) {
                     throw runtime_error("Invalid string operation: not a string address");
                }

                int newAddr = sptr_runtime;
                int start_of_str = strAddr;

                for (int i = 0; i < num; i++) {
                    strAddr = start_of_str; // 每次拷贝都重置到源字符串的开头
                    while(s[strAddr] != 0) {
                        if (sptr_runtime >= STRING_HEAP_MAX) throw overflow_error("Runtime string heap overflow");
                        s[sptr_runtime++] = s[strAddr++];
                    }
                }
                s[sptr_runtime++] = 0; // 空终止符
                s[t] = newAddr; // 将新字符串的地址压入栈
            }
            break;
            case 18:
            {
                int str2Addr = s[t];
                int str1Addr = s[t - 1];
                t = t - 1;

                if (str1Addr < STRING_CONSTANT_BASE || str1Addr > STRING_HEAP_MAX ||
                    str2Addr < STRING_CONSTANT_BASE || str2Addr > STRING_HEAP_MAX) {
                     throw runtime_error("Invalid string operation: not a string address");
                }

                int newAddr = sptr_runtime;
                // 复制第一个字符串
                int currentAddr = str1Addr;
                while(s[currentAddr] != 0) {
                    if (sptr_runtime >= STRING_HEAP_MAX) throw overflow_error("Runtime string heap overflow");
                    s[sptr_runtime++] = s[currentAddr++];
                }
                // 复制第二个字符串
                currentAddr = str2Addr;
                while(s[currentAddr] != 0) {
                    if (sptr_runtime >= STRING_HEAP_MAX) throw overflow_error("Runtime string heap overflow");
                    s[sptr_runtime++] = s[currentAddr++];
                }
                s[sptr_runtime++] = 0; // 空终止符
                s[t] = newAddr;
                break;
            }
            break;
            case 19:
            {
                int num = s[t];
                int strAddr = s[t-1];
                t = t - 1;

                if (strAddr < STRING_CONSTANT_BASE || strAddr > STRING_HEAP_MAX) {
                     throw runtime_error("Invalid string operation: not a string address");
                }
                
                std::string num_as_str = std::to_string(num);
                int newAddr = sptr_runtime;

                // 复制原始字符串
                int currentAddr = strAddr;
                while(s[currentAddr] != 0) {
                    if (sptr_runtime >= STRING_HEAP_MAX) throw overflow_error("Runtime string heap overflow");
                    s[sptr_runtime++] = s[currentAddr++];
                }
                // 复制数字字符串
                for(char c : num_as_str) {
                    if (sptr_runtime >= STRING_HEAP_MAX) throw overflow_error("Runtime string heap overflow");
                    s[sptr_runtime++] = static_cast<int>(c);
                }
                s[sptr_runtime++] = 0; // 空终止符
                s[t] = newAddr;
                break;
            }
            default:
                throw runtime_error("Unknown operation");
            }
            break;
        case LOD:
            t = t + 1;
            s[t] = s[base(inst.l, s, b) + inst.a];
            break;
        case STO:
            s[base(inst.l, s, b) + inst.a] = s[t];
            t = t - 1;
            break;
        case CAL:
            s[t + 1] = t;
            s[t + 2] = base(inst.l, s, b); // 静态链
            s[t + 3] = b;                  // 动态链
            s[t + 4] = p;                  // 返回地址
            b = t + 2;                     // 新基地址指向静态链的位置
            p = inst.a;                    // 跳转到函数入口
            t+=4;
            break;

        case INT:
            if (inst.a > 0 && t + inst.a > STACK_MAX) throw std::overflow_error("Stack overflow");
            t += inst.a;
            break;

        case JMP:
            p = inst.a;
            break;
        case JPC:
            if (s[t] == 0)
                p = inst.a;
            t = t - 1;
            break;
        case WRT:
        {
            int v = s[t];
            t = t - 1;
            // Check if the value is an address in our string heap
            if (v >= STRING_CONSTANT_BASE && v <= STRING_HEAP_MAX)
            {
                int addr = v;
                while (s[addr] != 0)
                {
                    cout << static_cast<char>(s[addr]);
                    fresult << static_cast<char>(s[addr]);
                    fas << static_cast<char>(s[addr]);
                    addr++;
                }
            }
            else
            {
                cout << v; // It's a number
                fresult << v; // It's a number
                fas << v;
            }
        }
        break;

        case RED:
            t = t + 1;
            cout << "\n";
            fresult << "\n";
            cin >> s[t];
            fresult << s[t] << endl; // 记录输入的值
            break;
        
        case LDA:
            if (t + 1 > STACK_MAX) throw std::overflow_error("Stack overflow"); // 检查栈溢出
            t = t + 1;
            s[t] = base(inst.l, s, b) + inst.a;
            break;

        case LDI:
            // s[t] 当前是一个地址, 我们要用这个地址指向的值来替换它
            s[t] = s[s[t]];
            break;
        case STI:
            // s[t] 是要存的值, s[t-1] 是目标地址
            s[s[t - 1]] = s[t];
            t = t - 2; // 将值和地址都弹出栈
            break;
        case HLT:
            return;
        }
    }
}

int Interpreter::base(int l, int *s, int b)
{
    int base = b;
    while (l > 0)
    {
        base = s[base];
        l--;
    }
    return base;
}
