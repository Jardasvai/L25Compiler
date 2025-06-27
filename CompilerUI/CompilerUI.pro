# Qt模块
QT += core gui widgets

# C++标准
CONFIG += c++17

# 关闭一些MSVC编译器的特定警告
msvc {
    QMAKE_CXXFLAGS += /wd4100 /wd4505
}

# 头文件搜索路径
INCLUDEPATH += ./compiler_backend

# UI源文件
SOURCES += \
    main.cpp \
    MainWindow.cpp \
    highlighter.cpp

# UI头文件
HEADERS += \
    MainWindow.h \
    highlighter.h

# 编译器后端源文件
SOURCES += \
    compiler_backend/Compiler.cpp \
    compiler_backend/Interpreter.cpp \
    compiler_backend/Parser.cpp \
    compiler_backend/Scanner.cpp \
    compiler_backend/table.cpp

# 编译器后端头文件 (qmake会自动处理，但明确列出更好)
HEADERS += \
    compiler_backend/Compiler.hpp \
    compiler_backend/Interpreter.hpp \
    compiler_backend/Parser.hpp \
    compiler_backend/Scanner.hpp \
    compiler_backend/Symbol.h \
    compiler_backend/table.hpp
RC_ICONS = L25Compiler.ico
