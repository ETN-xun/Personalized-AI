#include "customizepage.h"
#include <QHBoxLayout>
#include <QScreen>
#include <QApplication>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

CustomizePage::CustomizePage(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setupUI();
    
    // 设置窗口大小和位置
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        resize(500, 400);
        move((screenGeometry.width() - width()) / 2,
             (screenGeometry.height() - height()) / 2);
    }
}

CustomizePage::~CustomizePage()
{
}

void CustomizePage::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    
    // 标题栏
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLabel = new QLabel("量身定制", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    
    closeBtn = new QPushButton("×", this);
    closeBtn->setFixedSize(30, 30);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            color: #606060;
            font-size: 20px;
            background: transparent;
        }
        QPushButton:hover {
            color: red;
            background-color: rgba(255, 0, 0, 0.1);
        }
    )");
    
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(closeBtn);
    
    mainLayout->addLayout(titleLayout);
    
    // 添加文本展示框
    QTextEdit *instructionText = new QTextEdit(this);
    instructionText->setReadOnly(true);
    instructionText->setText("请填写您想要解决的问题，我将为您量身定制您的专属方案。");
    instructionText->setStyleSheet(R"(
        QTextEdit {
            background-color: #f5f5f5;
            border: 1px solid #ddd;
            border-radius: 5px;
            padding: 10px;
            font-size: 14px;
            color: #333;
        }
    )");
    mainLayout->addWidget(instructionText, 3); // 占3/4的空间
    
    // 底部输入区域布局
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    // 添加对话框（输入框）
    QLineEdit *inputField = new QLineEdit(this);
    inputField->setPlaceholderText("请在此输入您的问题...");
    inputField->setStyleSheet(R"(
        QLineEdit {
            background: white;
            border: 2px solid #4A90E2;
            border-radius: 5px;
            padding: 8px 15px;
            font-size: 14px;
        }
        QLineEdit:focus { 
            border-color: #63B8FF; 
        }
    )");
    
    // 添加确定按钮
    QPushButton *confirmBtn = new QPushButton("确定", this);
    confirmBtn->setStyleSheet(R"(
        QPushButton {
            background: #4A90E2;
            color: white;
            border-radius: 5px;
            font-size: 14px;
            min-width: 80px;
        }
        QPushButton:hover { 
            background: #63B8FF; 
        }
        QPushButton:pressed { 
            background: #3A7BFF; 
        }
    )");
    
    // 将输入框和确定按钮添加到输入布局
    inputLayout->addWidget(inputField, 4); // 输入框占4份宽度
    inputLayout->addWidget(confirmBtn, 1); // 确定按钮占1份宽度
    
    // 将输入布局添加到主布局，并设置高度为页面的1/4
    mainLayout->addLayout(inputLayout, 1); // 占1/4的空间
    
    // 连接确定按钮的点击信号
    connect(confirmBtn, &QPushButton::clicked, this, [this, inputField, instructionText]() {
        QString userInput = inputField->text().trimmed();
        if (!userInput.isEmpty()) {
            // 更新文本展示框内容，显示用户输入和回应
            instructionText->append("\n\n您的问题: " + userInput);
            instructionText->append("\n\n正在为您定制专属方案...");
            
            // 清空输入框
            inputField->clear();
            
            // 这里可以添加处理用户输入的逻辑
            // 例如发送到AI进行处理等
        }
    });
    
    // 连接回车键提交功能
    connect(inputField, &QLineEdit::returnPressed, confirmBtn, &QPushButton::click);
    
    // 连接关闭按钮信号
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    
    // 设置窗口样式
    setStyleSheet("background-color: white; border: 1px solid #ddd;");
}