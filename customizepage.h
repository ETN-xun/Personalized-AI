#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QNetworkAccessManager>

class CustomizePage : public QWidget
{
    Q_OBJECT

public:
    explicit CustomizePage(QWidget *parent = nullptr);
    ~CustomizePage();

private slots:
    void onConfirmButtonClicked();
    void onGenerateScheduleClicked(); // 添加新的槽函数

private:
    void setupUI();
    void sendDeepseekRequest(const QString &prompt);
    
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeBtn;
    QTextEdit *instructionText;
    QLineEdit *inputField;
    QPushButton *confirmBtn;
    QPushButton *generateScheduleBtn; // 添加计划表按钮
    
    QNetworkAccessManager *networkManager;
    QString apiKey;
    
    bool isFirstQuestion;
    QString originalQuestion;
    QString userInfo; // 添加存储用户基本信息的变量
    bool hasGeneratedSolution; // 添加标记是否已生成解决方案
};