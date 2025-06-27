#include <vector>
#include <stdexcept>
#include "Symbol.h" 

class SymSet {
private:
    std::vector<bool> bits; // 使用 vector<bool> 存储位集合

public:
    /**
     * 构造一个符号集合
     * 
     * @param nbits 集合的初始容量
     */
    explicit SymSet(size_t nbits = 0) : bits(nbits, false) {}

    /**
     * 设置指定符号对应的位
     * 
     * @param s 要设置的符号
     */
    void set(Symbol s) {
        size_t pos = static_cast<size_t>(s);
        if (pos >= bits.size()) {
            bits.resize(pos + 1, false); // 动态扩展位集
        }
        bits[pos] = true;
    }

    /**
     * 检查符号是否在集合中
     * 
     * @param s 要检查的符号
     * @return 若符号在集合中返回 true，否则 false
     */
    bool get(Symbol s) const {
        size_t pos = static_cast<size_t>(s);
        return (pos < bits.size()) ? bits[pos] : false;
    }
};