#pragma once

#include <QString>
#include <QTextDocument>
#include <QRegularExpression>

class MarkdownParser {
public:
    static QString toHtml(const QString &markdown);

private:
    static QString processInlineFormats(QString text);
    static QString processCodeBlocks(QString text);
    static QString processBulletLists(QString text);
    static QString processNumberedLists(QString text);
    static QString processHeadings(QString text);
    static QString processBlockquotes(QString text); // 新增：处理引用块
    static QString processHorizontalRules(QString text); // 新增：处理水平分割线
    static QString processLinks(QString text); // 新增：处理链接
    static QString processTables(QString text); // 新增：处理表格
    static QString processTaskLists(QString text); // 新增：处理任务列表
};