#include "highlighter.h"

highlighter::highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // 1. 关键字高亮
    keywordFormat.setForeground(QColor("#CF8E6D")); // 橙色
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bprogram\\b" << "\\bmain\\b" << "\\blet\\b"
                    << "\\bstr\\b" << "\\bif\\b" << "\\belse\\b"
                    << "\\bwhile\\b" << "\\bfunc\\b" << "\\breturn\\b"
                    << "\\binput\\b" << "\\boutput\\b";

    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 2. 数字高亮
    numberFormat.setForeground(QColor("#B5CEA8")); // 绿色
    rule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 3. 字符串高亮
    quotationFormat.setForeground(QColor("#CE9178")); // 赭色
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 4. 函数名高亮 (后面跟着'(')
    functionFormat.setForeground(QColor("#DCDCAA")); // 淡黄色
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // 5. 注释高亮 (假设L25使用'//'作为单行注释)
    singleLineCommentFormat.setForeground(QColor("#6A9955")); // 深绿色
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 6. 符号/操作符高亮
    operatorFormat.setForeground(QColor("#D4D4D4")); // 灰色
    rule.pattern = QRegularExpression("[\\+\\-\\*/=&|@!<>\\(\\)\\{\\},;]");
    rule.format = operatorFormat;
    highlightingRules.append(rule);
}

void highlighter::highlightBlock(const QString &text)
{
    // 依次应用所有高亮规则
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
