#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// 前向声明Qt控件和我们的自定义类
class QPlainTextEdit;
class QPushButton;
class QTabWidget;
class QLabel;
class highlighter;
class Compiler;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void saveFile();
    void compileCode();
    void runCode();
    void updateLineColInfo();

private:
    // 初始化函数
    void createWidgets();
    void createLayout();
    void createConnections();
    void createToolbar();
    void createStatusBar();

    // 编译器交互函数
    void resetStreams();
    void displayCompilerOutput();
    void setInitialCode();

    // UI 控件
    QPlainTextEdit *sourceEdit;
    QPlainTextEdit *inputEdit;
    QPlainTextEdit *runtimeOutputEdit; // 专门用于运行时输出

    QTabWidget *infoTab;
    QPlainTextEdit *messagesEdit;        // 编译信息
    QPlainTextEdit *intermediateCodeEdit;// P-Code
    QPlainTextEdit *symbolTableEdit;     // 符号表

    QPushButton *compileButton;
    QPushButton *runButton;

    QLabel *lineLabel;
    QLabel *statusLabel;

    // 自定义类实例
    highlighter *highlighter;
    Compiler *l25Compiler = nullptr;

    // 状态变量
    QString currentFile;
    bool isCompiled = false;
};

#endif // MAINWINDOW_H
