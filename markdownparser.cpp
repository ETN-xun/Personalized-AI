#include "markdownparser.h"

QString MarkdownParser::toHtml(const QString &markdown) {
    QString html = markdown;
    
    // 处理代码块
    html = processCodeBlocks(html);
    
    // 处理水平分割线
    html = processHorizontalRules(html);
    
    // 处理引用块
    html = processBlockquotes(html);
    
    // 处理表格
    html = processTables(html);
    
    // 处理标题
    html = processHeadings(html);
    
    // 处理列表
    html = processBulletLists(html);
    html = processNumberedLists(html);
    html = processTaskLists(html);
    
    // 处理链接
    html = processLinks(html);
    
    // 处理行内格式（粗体、斜体等）
    html = processInlineFormats(html);
    
    // 处理换行
    html.replace("\n", "<br>");
    
    return html;
}

QString MarkdownParser::processInlineFormats(QString text) {
    // 处理粗体 (**text** 或 __text__)
    QRegularExpression boldRegex("\\*\\*(.*?)\\*\\*|__(.*?)__");
    QRegularExpressionMatchIterator boldMatches = boldRegex.globalMatch(text);
    while (boldMatches.hasNext()) {
        QRegularExpressionMatch match = boldMatches.next();
        QString matched = match.captured(0);
        QString content = match.captured(1).isEmpty() ? match.captured(2) : match.captured(1);
        text.replace(matched, "<b>" + content + "</b>");
    }
    
    // 处理斜体 (*text* 或 _text_)
    QRegularExpression italicRegex("\\*(.*?)\\*|_(.*?)_");
    QRegularExpressionMatchIterator italicMatches = italicRegex.globalMatch(text);
    while (italicMatches.hasNext()) {
        QRegularExpressionMatch match = italicMatches.next();
        QString matched = match.captured(0);
        QString content = match.captured(1).isEmpty() ? match.captured(2) : match.captured(1);
        text.replace(matched, "<i>" + content + "</i>");
    }
    
    // 处理删除线 (~~text~~)
    QRegularExpression strikeRegex("~~(.*?)~~");
    QRegularExpressionMatchIterator strikeMatches = strikeRegex.globalMatch(text);
    while (strikeMatches.hasNext()) {
        QRegularExpressionMatch match = strikeMatches.next();
        text.replace(match.captured(0), "<s>" + match.captured(1) + "</s>");
    }
    
    // 处理行内代码 (`code`)
    QRegularExpression codeRegex("`([^`]*?)`");
    QRegularExpressionMatchIterator codeMatches = codeRegex.globalMatch(text);
    while (codeMatches.hasNext()) {
        QRegularExpressionMatch match = codeMatches.next();
        text.replace(match.captured(0), "<code style='background-color:#f0f0f0;padding:2px;border-radius:3px;font-family:monospace;'>" + match.captured(1) + "</code>");
    }
    
    return text;
}

QString MarkdownParser::processCodeBlocks(QString text) {
    // 处理代码块 (```code```)
    QRegularExpression codeBlockRegex("```(?:.*?)\\n([\\s\\S]*?)```");
    QRegularExpressionMatchIterator matches = codeBlockRegex.globalMatch(text);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString codeContent = match.captured(1);
        QString replacement = "<pre style='background-color:#f0f0f0;padding:10px;border-radius:5px;font-family:monospace;'>" + codeContent + "</pre>";
        text.replace(match.captured(0), replacement);
    }
    return text;
}

QString MarkdownParser::processBulletLists(QString text) {
    // 处理无序列表
    QRegularExpression listRegex("((?:^\\s*[-*+]\\s+.*?$\\n?)+)", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator matches = listRegex.globalMatch(text);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString listContent = match.captured(1);
        
        // 将每一行转换为列表项
        QRegularExpression itemRegex("^\\s*[-*+]\\s+(.*?)$", QRegularExpression::MultilineOption);
        QString htmlList = "<ul style='margin-left:20px;'>\n";
        QRegularExpressionMatchIterator itemMatches = itemRegex.globalMatch(listContent);
        while (itemMatches.hasNext()) {
            QRegularExpressionMatch itemMatch = itemMatches.next();
            htmlList += "<li>" + itemMatch.captured(1) + "</li>\n";
        }
        htmlList += "</ul>";
        
        text.replace(match.captured(0), htmlList);
    }
    return text;
}

QString MarkdownParser::processNumberedLists(QString text) {
    // 处理有序列表
    QRegularExpression listRegex("((?:^\\s*\\d+\\.\\s+.*?$\\n?)+)", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator matches = listRegex.globalMatch(text);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString listContent = match.captured(1);
        
        // 将每一行转换为列表项
        QRegularExpression itemRegex("^\\s*\\d+\\.\\s+(.*?)$", QRegularExpression::MultilineOption);
        QString htmlList = "<ol style='margin-left:20px;'>\n";
        QRegularExpressionMatchIterator itemMatches = itemRegex.globalMatch(listContent);
        while (itemMatches.hasNext()) {
            QRegularExpressionMatch itemMatch = itemMatches.next();
            htmlList += "<li>" + itemMatch.captured(1) + "</li>\n";
        }
        htmlList += "</ol>";
        
        text.replace(match.captured(0), htmlList);
    }
    return text;
}

QString MarkdownParser::processHeadings(QString text) {
    // 处理标题 (# Heading)
    for (int i = 6; i >= 1; i--) {
        QString pattern = "^" + QString(i, '#') + "\\s+(.+?)$";
        QRegularExpression headingRegex(pattern, QRegularExpression::MultilineOption);
        QRegularExpressionMatchIterator matches = headingRegex.globalMatch(text);
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            QString heading = match.captured(1);
            text.replace(match.captured(0), QString("<h%1>%2</h%1>").arg(i).arg(heading));
        }
    }
    return text;
}

// 新增：处理引用块
QString MarkdownParser::processBlockquotes(QString text) {
    QRegularExpression blockquoteRegex("((?:^>\\s+.*?$\\n?)+)", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator matches = blockquoteRegex.globalMatch(text);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString quoteContent = match.captured(1);
        
        // 移除每行开头的引用符号
        QRegularExpression lineRegex("^>\\s+(.*?)$", QRegularExpression::MultilineOption);
        QString processedContent;
        QRegularExpressionMatchIterator lineMatches = lineRegex.globalMatch(quoteContent);
        while (lineMatches.hasNext()) {
            QRegularExpressionMatch lineMatch = lineMatches.next();
            processedContent += lineMatch.captured(1) + "\n";
        }
        
        QString replacement = "<blockquote style='border-left:4px solid #ccc;padding-left:15px;color:#777;margin:10px 0;'>" 
                            + processedContent.trimmed() + "</blockquote>";
        text.replace(match.captured(0), replacement);
    }
    return text;
}

// 新增：处理水平分割线
QString MarkdownParser::processHorizontalRules(QString text) {
    QRegularExpression hrRegex("^(\\s*[*\\-_]\\s*){3,}$", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator matches = hrRegex.globalMatch(text);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        text.replace(match.captured(0), "<hr style='border:0;border-top:1px solid #ccc;margin:10px 0;'>");
    }
    return text;
}

// 新增：处理链接
QString MarkdownParser::processLinks(QString text) {
    // 处理 [text](url) 格式的链接
    QRegularExpression linkRegex("\\[(.*?)\\]\\((.*?)\\)");
    QRegularExpressionMatchIterator matches = linkRegex.globalMatch(text);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        text.replace(match.captured(0), QString("<a href='%1' style='color:#4A90E2;text-decoration:none;'>%2</a>").arg(url, linkText));
    }
    
    // 处理自动链接 <http://example.com>
    QRegularExpression autoLinkRegex("<(https?://[^>]+)>");
    QRegularExpressionMatchIterator autoMatches = autoLinkRegex.globalMatch(text);
    while (autoMatches.hasNext()) {
        QRegularExpressionMatch match = autoMatches.next();
        QString url = match.captured(1);
        text.replace(match.captured(0), QString("<a href='%1' style='color:#4A90E2;text-decoration:none;'>%1</a>").arg(url));
    }
    
    return text;
}

// 新增：处理表格
QString MarkdownParser::processTables(QString text) {
    // 匹配整个表格，包括表头和分隔行
    QRegularExpression tableRegex("^\\|(.+)\\|\\s*\\n\\|\\s*[-:]+[-\\s|:]*[-:]\\s*\\|\\s*\\n((\\|.+\\|\\s*\\n)+)",
                                QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator matches = tableRegex.globalMatch(text);
    
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString headerRow = match.captured(1);
        QString bodyRows = match.captured(2);
        
        // 处理表头
        QStringList headers;
        QRegularExpression cellRegex("\\|\\s*([^|]+)\\s*");
        QRegularExpressionMatchIterator cellMatches = cellRegex.globalMatch("|" + headerRow + "|");
        while (cellMatches.hasNext()) {
            QRegularExpressionMatch cellMatch = cellMatches.next();
            headers << cellMatch.captured(1).trimmed();
        }
        
        // 创建HTML表格
        QString tableHtml = "<table style='border-collapse:collapse;width:100%;margin:10px 0;'>\n<thead>\n<tr>\n";
        for (const QString &header : headers) {
            tableHtml += QString("<th style='border:1px solid #ddd;padding:8px;text-align:left;background-color:#f2f2f2;'>%1</th>\n").arg(header);
        }
        tableHtml += "</tr>\n</thead>\n<tbody>\n";
        
        // 处理表格内容行
        QRegularExpression rowRegex("^\\|(.+)\\|\\s*$", QRegularExpression::MultilineOption);
        QRegularExpressionMatchIterator rowMatches = rowRegex.globalMatch(bodyRows);
        while (rowMatches.hasNext()) {
            QRegularExpressionMatch rowMatch = rowMatches.next();
            QString rowContent = rowMatch.captured(1);
            
            tableHtml += "<tr>\n";
            QRegularExpressionMatchIterator cellMatches = cellRegex.globalMatch("|" + rowContent + "|");
            while (cellMatches.hasNext()) {
                QRegularExpressionMatch cellMatch = cellMatches.next();
                tableHtml += QString("<td style='border:1px solid #ddd;padding:8px;'>%1</td>\n").arg(cellMatch.captured(1).trimmed());
            }
            tableHtml += "</tr>\n";
        }
        
        tableHtml += "</tbody>\n</table>";
        text.replace(match.captured(0), tableHtml);
    }
    
    return text;
}

// 新增：处理任务列表
QString MarkdownParser::processTaskLists(QString text) {
    // 匹配任务列表项 - [ ] 或 - [x]
    QRegularExpression taskListRegex("^\\s*[-*+]\\s+\\[([ xX])\\]\\s+(.+?)$", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator matches = taskListRegex.globalMatch(text);
    
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString status = match.captured(1);
        QString content = match.captured(2);
        
        bool checked = (status == "x" || status == "X");
        QString replacement = QString("<div style='margin:5px 0;'><input type='checkbox' %1 disabled> %2</div>")
                            .arg(checked ? "checked" : "", content);
        
        text.replace(match.captured(0), replacement);
    }
    
    return text;
}