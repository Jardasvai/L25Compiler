#include "Compiler.hpp"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
ifstream fin;
ofstream ftable;
ofstream fcode;
ofstream fas;
ofstream fresult;
int main()
{
    string fname;
    int err_cnt; // count for errors
    string input_option;
    char ch;
    cout << ("Input L25 file path:") << endl;
    cin >> fname;
    fin.open(fname);
    if (!fin)
    {
        cerr << "The input file cannot be opened: " << fname << endl;
        exit(1);
    }

    // 检查文件是否为空
    fin.seekg(0, ios::end);
    if (fin.tellg() == 0)
    {
        cout << "The input file is empty!" << endl;
        fin.close();
        exit(1);
    }
    fin.seekg(0, ios::beg);

    ftable.open("ftable.txt");
    if (!ftable)
    {
        cerr << "The table file cannot be opened!" << endl;
        exit(1);
    }

    cout << "List object codes?(Y/N)" << endl;
    cin >> input_option;
    listswitch = (input_option == "y" || input_option == "Y");
    cout << listswitch << endl;

    cout << "List symbol table?(Y/N)" << endl;
    cin >> input_option;
    tableswitch = (input_option == "y" || input_option == "Y");
    cout << tableswitch << endl;    // 设置全局输出文件流
    fas.open("foutput.txt");
    if (!fas)
    {
        cerr << "Cannot open output file for general output!" << endl;
        exit(1);
    }
    
    // 设置代码文件流
    fcode.open("fcode.txt");
    if (!fcode)
    {
        cerr << "Cannot open output file for P-code instructions!" << endl;
        exit(1);
    }
    
    // 设置结果文件流
    fresult.open("fresult.txt");
    if (!fresult)
    {
        cerr << "Cannot open output file for execution results!" << endl;
        exit(1);
    }
    // 创建编译器并编译
    Compiler L25compiler(fin);
    bool result = L25compiler.compile();

    if (result)
    {
        cout << "Compilation successful!" << endl;

        // 询问是否执行程序
        cout << "Execute the program?(Y/N)" << endl;
        cin >> input_option;
        bool executeswitch = (input_option == "y" || input_option == "Y");

        if (executeswitch)
        {
            cout << "\n=== Program Output ===" << endl;
            L25compiler.execute();
        }
    }
    else
    {
        cout << "Compilation failed!" << endl;
    }

    fin.close();
    fas.close();
    fcode.close();
    fresult.close();
    ftable.close();

    return 0;
}
