#pragma once
#include "Compiler.hpp"
#include <unordered_map>
#include <string>

#define stacksize 5000 // largest number of the elements in stack while running
#define STACK_MAX 999

//define memory region
#define STRING_CONSTANT_BASE 1000
#define STRING_RUNTIME_BASE 2000
#define STRING_HEAP_MAX 4999

enum Fct
{
    LIT,
    OPR,
    LOD,
    STO,
    CAL,
    INT,
    JMP,
    JPC,
    WRT,
    RED,
    HLT,
    LDA,
    LDI,
    STI
};

class Instruction
{
public:
    Fct f; // Function code
    int l; // Level
    int a; // Address or value

    Instruction(Fct function = LIT, int level = 0, int address = 0)
        : f(function), l(level), a(address) {}
};

class Interpreter
{
private:
    int base(int l, int *s, int b); // Base address calculation
    /* data */
public:
    // static const int STRING_BASE = 1000;                    // starting address for string storage
    // static int sptr;                                        // next free address in string storage
    // static std::unordered_map<int, std::string> stringHeap; // string storage
    int sptr_const;
    int sptr_runtime;
    int s[stacksize];       //虚拟机内存空间
    Interpreter():cx(0), sptr_const(STRING_CONSTANT_BASE), sptr_runtime(STRING_RUNTIME_BASE) {}
    void genString(const std::string &s); // generate string literal

    int cx = 0;              // Instruction pointer
    Instruction code[cxmax]; // Code array

    void gen(Fct x, int y, int z); // Generate code
    void listCode(int cx0);        // List code for debugging
    void interpret();              // Start interpretation
    void setCode(int addr, Fct f, int l, int a)
    { // Set code at specific address
        if (addr >= 0 && addr < cxmax)
        {
            code[addr] = Instruction(f, l, a);
        }
    }
};
