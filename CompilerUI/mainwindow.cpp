#include "MainWindow.h"
#include "highlighter.h"
#include "compiler_backend/Compiler.hpp" // 引入你的编译器
#include <sstream>

#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFontDatabase>
#include <QDateTime>
// 引用在main.cpp中定义的全局变量
extern std::ostringstream fas_stream, fcode_stream, ftable_stream, fresult_stream;
extern std::istringstream* g_inputStream;
extern bool listswitch, tableswitch;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置主窗口
    setWindowTitle("L25 编译器 IDE");
    resize(1200, 800);
    setStyleSheet("QMainWindow { background-color: #2D2D2D; color: #D4D4D4; }"
                  "QGroupBox { font-weight: bold; }");

    // 调用初始化函数
    createWidgets();
    createLayout();
    createConnections();
    createToolbar();
    createStatusBar();
    setInitialCode();
}

MainWindow::~MainWindow()
{
    delete l25Compiler; // 清理编译器实例
}

void MainWindow::createWidgets()
{
    // --- 代码编辑器 ---
    sourceEdit = new QPlainTextEdit();
    QFont editorFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    editorFont.setPointSize(11);
    sourceEdit->setFont(editorFont);
    sourceEdit->setStyleSheet("background-color: #1E1E1E; color: #D4D4D4; border: 1px solid #3c3c3c;");
    highlighter = new class highlighter(sourceEdit->document());
    // --- 输入/输出区域 ---
    inputEdit = new QPlainTextEdit();
    inputEdit->setFont(editorFont);
    inputEdit->setPlaceholderText("在此输入程序运行时需要的数据...");
    inputEdit->setStyleSheet("background-color: #1E1E1E; color: #D4D4D4; border: 1px solid #3c3c3c;");

    runtimeOutputEdit = new QPlainTextEdit();
    runtimeOutputEdit->setReadOnly(true);
    runtimeOutputEdit->setFont(editorFont);
    runtimeOutputEdit->setStyleSheet("background-color: #1E1E1E; color: #D4D4D4; border: 1px solid #3c3c3c;");

    // --- 编译器信息Tab ---
    infoTab = new QTabWidget();
    infoTab->setStyleSheet("QTabBar::tab { background: #3c3c3c; color: #D4D4D4; padding: 5px; }"
                           "QTabBar::tab:selected { background: #007ACC; }"
                           "QTabWidget::pane { border: 1px solid #3c3c3c; }");

    messagesEdit = new QPlainTextEdit();
    intermediateCodeEdit = new QPlainTextEdit();
    symbolTableEdit = new QPlainTextEdit();

    messagesEdit->setReadOnly(true);
    intermediateCodeEdit->setReadOnly(true);
    symbolTableEdit->setReadOnly(true);

    QFont infoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    infoFont.setPointSize(10);
    messagesEdit->setFont(infoFont);
    intermediateCodeEdit->setFont(infoFont);
    symbolTableEdit->setFont(infoFont);

    messagesEdit->setStyleSheet("background-color: #252526; color: #D4D4D4;");
    intermediateCodeEdit->setStyleSheet("background-color: #252526; color: #D4D4D4;");
    symbolTableEdit->setStyleSheet("background-color: #252526; color: #D4D4D4;");

    infoTab->addTab(messagesEdit, "编译信息");
    infoTab->addTab(intermediateCodeEdit, "P-Code");
    infoTab->addTab(symbolTableEdit, "符号表");

    // --- 按钮 ---
    compileButton = new QPushButton("编译");
    runButton = new QPushButton("运行");

    QString buttonStyle = "QPushButton { background-color: #007ACC; color: white; border: none; padding: 8px; font-weight: bold; }"
                          "QPushButton:hover { background-color: #005A9E; }"
                          "QPushButton:pressed { background-color: #004578; }";
    compileButton->setStyleSheet(buttonStyle);
    runButton->setStyleSheet(buttonStyle);
}

void MainWindow::createLayout()
{
    // --- 主布局，使用QSplitter实现可拖动调整大小 ---
    QSplitter *mainSplitter = new QSplitter(Qt::Vertical);

    // 1. 顶部区域：代码编辑器
    QGroupBox *sourceGroup = new QGroupBox("源代码");
    QVBoxLayout *sourceLayout = new QVBoxLayout(sourceGroup);
    sourceLayout->addWidget(sourceEdit);
    mainSplitter->addWidget(sourceGroup);

    // 2. 中部区域：运行时输入/输出
    QSplitter *ioSplitter = new QSplitter(Qt::Horizontal);
    QGroupBox *inputGroup = new QGroupBox("程序输入");
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->addWidget(inputEdit);
    ioSplitter->addWidget(inputGroup);

    QGroupBox *runtimeOutputGroup = new QGroupBox("运行时输出");
    QVBoxLayout *runtimeOutputLayout = new QVBoxLayout(runtimeOutputGroup);
    runtimeOutputLayout->addWidget(runtimeOutputEdit);
    ioSplitter->addWidget(runtimeOutputGroup);
    mainSplitter->addWidget(ioSplitter);

    // 3. 底部区域：编译器信息
    QGroupBox *infoGroup = new QGroupBox("编译器信息");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    infoLayout->addWidget(infoTab);
    mainSplitter->addWidget(infoGroup);

    // 设置初始大小比例
    mainSplitter->setSizes({500, 200, 200});

    // 创建一个中心Widget并设置主布局
    QWidget *centralWidget = new QWidget();
    QHBoxLayout *centralLayout = new QHBoxLayout(centralWidget);
    centralLayout->addWidget(mainSplitter);
    setCentralWidget(centralWidget);
}

void MainWindow::createConnections()
{
    // 连接按钮点击事件
    connect(compileButton, &QPushButton::clicked, this, &MainWindow::compileCode);
    connect(runButton, &QPushButton::clicked, this, &MainWindow::runCode);
    // 连接光标位置变化事件
    connect(sourceEdit, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateLineColInfo);
}

void MainWindow::createToolbar()
{
    QToolBar *toolbar = addToolBar("主工具栏");
    toolbar->setMovable(false);

    QAction *openAction = toolbar->addAction("打开");
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    QAction *saveAction = toolbar->addAction("保存");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);

    toolbar->addSeparator();
    toolbar->addWidget(compileButton);
    toolbar->addWidget(runButton);
}

void MainWindow::createStatusBar()
{
    statusBar()->setStyleSheet("background-color: #007ACC; color: white;");
    statusBar()->showMessage("就绪");
    lineLabel = new QLabel("行: 1, 列: 1");
    statusBar()->addPermanentWidget(lineLabel);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "打开L25源文件", "", "L25 Files (*.l25);;All Files (*.*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件: " + fileName);
        return;
    }

    currentFile = fileName;
    QTextStream in(&file);
    sourceEdit->setPlainText(in.readAll());
    file.close();
    statusBar()->showMessage("已打开: " + currentFile, 5000);
}

void MainWindow::saveFile()
{
    QString fileName = currentFile;
    if (fileName.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(this, "保存L25源文件", "", "L25 Files (*.l25);;All Files (*.*)");
        if (fileName.isEmpty()) return;
        currentFile = fileName;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法保存文件: " + fileName);
        return;
    }

    QTextStream out(&file);
    out << sourceEdit->toPlainText();
    file.close();
    statusBar()->showMessage("文件已保存: " + currentFile, 3000);
}


void MainWindow::compileCode()
{
    // 1. 清理
    if (l25Compiler) delete l25Compiler;
    resetStreams();
    messagesEdit->clear();
    intermediateCodeEdit->clear();
    symbolTableEdit->clear();
    statusBar()->showMessage("编译中...");

    // 2. 获取源码
    std::string sourceCode = sourceEdit->toPlainText().toStdString();
    if (sourceCode.empty()) {
        QMessageBox::warning(this, "警告", "源代码为空，无法编译。");
        statusBar()->showMessage("就绪");
        return;
    }

    // 3. 设置编译选项 (总是生成所有信息，由UI决定是否显示)
    listswitch = true;
    tableswitch = true;

    // 4. 执行编译
    l25Compiler = new Compiler(sourceCode);
    isCompiled = l25Compiler->compile();

    // 5. 显示结果
    displayCompilerOutput();

    if (isCompiled) {
        messagesEdit->appendPlainText("\n--- 编译成功 ---");
        statusBar()->showMessage("编译成功", 5000);
    } else {
        messagesEdit->appendPlainText("\n--- 编译失败 ---");
        statusBar()->showMessage("编译失败", 5000);
    }
}

void MainWindow::runCode()
{
    if (!isCompiled || !l25Compiler) {
        QMessageBox::critical(this, "错误", "请先成功编译代码。");
        return;
    }

    runtimeOutputEdit->clear();
    resetStreams();
    statusBar()->showMessage("运行中...");
    // 1. 清理上一次运行的输入流 (防止内存泄漏)
    delete g_inputStream;

    // 2. 从UI获取所有输入，并用它创建一个新的全局输入流
    std::string all_input = inputEdit->toPlainText().toStdString();
    g_inputStream = new std::istringstream(all_input);

    bool execution_ok = l25Compiler->execute();

    std::string program_output = fresult_stream.str();
    runtimeOutputEdit->setPlainText(QString::fromStdString(program_output));

    if (execution_ok) {
        runtimeOutputEdit->appendPlainText("\n--- 程序执行完毕 ---");
        statusBar()->showMessage("运行完毕", 5000);
    } else {
        runtimeOutputEdit->appendPlainText("\n--- 程序执行出错 ---");
        statusBar()->showMessage("运行出错", 5000);
    }
}


void MainWindow::updateLineColInfo()
{
    QTextCursor cursor = sourceEdit->textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.columnNumber() + 1;
    lineLabel->setText(QString("行: %1, 列: %2").arg(line).arg(column));
}

void MainWindow::resetStreams()
{
    fas_stream.str("");
    fas_stream.clear();
    fcode_stream.str("");
    fcode_stream.clear();
    ftable_stream.str("");
    ftable_stream.clear();
    fresult_stream.str("");
    fresult_stream.clear();
}

void MainWindow::displayCompilerOutput()
{
    messagesEdit->setPlainText(QString::fromStdString(fas_stream.str()));
    intermediateCodeEdit->setPlainText(QString::fromStdString(fcode_stream.str()));
    symbolTableEdit->setPlainText(QString::fromStdString(ftable_stream.str()));
}

void MainWindow::setInitialCode()
{
    sourceEdit->setPlainText(
        "program MyApp {\n"
        "    func add(a, b) {\n"
        "        let sum = a + b;\n"
        "        return sum;\n"
        "    }\n"
        "    func square(x) {\n"
        "        let result = x * x;\n"
        "        return result;\n"
        "    }\n"
        "    main {\n"
        "        str s = \"adasdf\";\n"
        "        let x;\n"
        "        x = 5;\n"
        "        let y = add(x, 10);\n"
        "        let z = square(y);\n"
        "        if (z > 50) {\n"
        "            output(z);\n"
        "        } else {\n"
        "            output(0);\n"
        "        };\n"
        "    }\n"
        "}\n"
        );
}
