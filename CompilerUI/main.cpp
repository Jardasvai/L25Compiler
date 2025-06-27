#include "MainWindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <sstream>

// --- 全局变量定义区 ---
// 这里是全局流和开关的唯一定义位置

// 定义输出流对象
std::ostringstream fas_stream;
std::ostringstream fcode_stream;
std::ostringstream ftable_stream;
std::ostringstream fresult_stream;

// 定义输入流指针 (初始为nullptr)
std::istringstream* g_inputStream = nullptr;

// 定义全局开关
bool listswitch = false;
bool tableswitch = false;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    MainWindow w;
    w.show();

    int result = a.exec();

    // 在程序退出前，确保全局输入流被删除
    delete g_inputStream;
    g_inputStream = nullptr;

    return result;
}
