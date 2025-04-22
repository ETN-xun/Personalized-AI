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

private:
    void setupUI();
    void sendDeepseekRequest(const QString &prompt);
    
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeBtn;
    QTextEdit *instructionText;
    QLineEdit *inputField;
    QPushButton *confirmBtn;
    
    QNetworkAccessManager *networkManager;
    QString apiKey;
    
    // 添加状态变量和存储原始问题的变量
    bool isFirstQuestion;
    QString originalQuestion;
};