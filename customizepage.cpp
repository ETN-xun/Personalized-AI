#include "customizepage.h"
#include <QHBoxLayout>
#include <QScreen>
#include <QApplication>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QScrollBar> // 添加滚动条头文件，用于滚动到底部


CustomizePage::CustomizePage(QWidget *parent)
    : QWidget(parent),
    networkManager(new QNetworkAccessManager(this)),
    apiKey("sk-eea6568b51c74da88e91f32f91485ab9"), // 使用与主窗口相同的API密钥
    isFirstQuestion(true), // 初始化为第一次提问
    hasGeneratedSolution(false) // 初始化为未生成解决方案
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
    delete networkManager;
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
    instructionText = new QTextEdit(this); // 修改为类成员变量
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
    inputField = new QLineEdit(this); // 修改为类成员变量
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
    confirmBtn = new QPushButton("确定", this); // 修改为类成员变量
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
    
    // 添加"一键生成计划表"按钮（初始隐藏）
    generateScheduleBtn = new QPushButton("一键生成计划表", this);
    generateScheduleBtn->setStyleSheet(R"(
        QPushButton {
            background: #4A90E2;
            color: white;
            border-radius: 5px;
            font-size: 14px;
            padding: 8px 15px;
            margin-top: 10px;
        }
        QPushButton:hover { 
            background: #63B8FF; 
        }
        QPushButton:pressed { 
            background: #3A7BFF; 
        }
    )");
    generateScheduleBtn->hide(); // 初始隐藏按钮
    
    // 将按钮添加到主布局
    mainLayout->addWidget(generateScheduleBtn, 0, Qt::AlignCenter);
    
    // 连接按钮信号
    connect(generateScheduleBtn, &QPushButton::clicked, this, &CustomizePage::onGenerateScheduleClicked);
    
    // 连接确定按钮的点击信号
    connect(confirmBtn, &QPushButton::clicked, this, &CustomizePage::onConfirmButtonClicked);
    
    // 连接回车键提交功能
    connect(inputField, &QLineEdit::returnPressed, confirmBtn, &QPushButton::click);
    
    // 连接关闭按钮信号
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    
    // 设置窗口样式
    setStyleSheet("background-color: white; border: 1px solid #ddd;");
}

void CustomizePage::onConfirmButtonClicked()
{
    QString userInput = inputField->text().trimmed();
    if (userInput.isEmpty()) return;
    
    // 清空输入框
    inputField->clear();
    
    // 禁用确定按钮，防止重复提交
    confirmBtn->setEnabled(false);
    
    if (isFirstQuestion) {
        // 第一次提问，保存原始问题
        originalQuestion = userInput;
        
        // 显示正在处理的提示
        instructionText->setText("正在为您定制专属方案，请稍候...");
        
        // 构建第一次请求的提示
        QString prompt = QString("我遇到了%1的问题，现在想要你帮我解决，但因为不同的人需要不同的有针对性的方针，"
                                "所以我希望你能向我提几个问题来收集解决这个问题所需的基本信息，"
                                "来为我制定个性化的解决方案。你需要将问题一次性全部问出").arg(userInput);
        
        // 发送第一次请求
        sendDeepseekRequest(prompt);
        
        // 更新状态为第二次提问
        isFirstQuestion = false;
        
        // 隐藏生成计划表按钮（如果之前显示）
        generateScheduleBtn->hide();
        hasGeneratedSolution = false;
    } else {
        // 第二次提问，用户已提供基本信息
        userInfo = userInput; // 保存用户基本信息
        
        // 显示正在处理的提示
        instructionText->setText("正在根据您的信息生成个性化方案，请稍候...");
        
        // 构建第二次请求的提示
        QString prompt = QString("我遇到了%1的问题，现在想要你帮我解决，但因为不同的人需要不同的有针对性的方针，"
                                "以下是我的基本信息：%2。请你根据我的基本信息来为我制定一套个性化方案")
                                .arg(originalQuestion, userInput);
        
        // 发送第二次请求
        sendDeepseekRequest(prompt);
        
        // 重置状态为第一次提问（以便用户可以开始新的咨询）
        isFirstQuestion = true;
        
        // 标记已生成解决方案
        hasGeneratedSolution = true;
    }
}

void CustomizePage::sendDeepseekRequest(const QString &prompt)
{
    // 构建API请求
    QJsonObject json{
        {"model", "deepseek-chat"},
        {"temperature", 0.7},
        {"max_tokens", 2048},
    };
    
    // 构建消息数组
    QJsonArray messages = {
        QJsonObject{
            {"role", "system"}, 
            {"content", "你是一个专业的问题解决顾问，擅长针对不同用户的需求提供个性化的解决方案和计划表。"}
        },
        QJsonObject{
            {"role", "user"}, 
            {"content", prompt}
        }
    };
    
    json["messages"] = messages;
    
    // 创建网络请求
    QNetworkRequest request(QUrl("https://api.deepseek.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());
    
    // 发送请求
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    
    // 处理响应
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // 重新启用确定按钮
        confirmBtn->setEnabled(true);
        
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isObject() && doc.object().contains("choices")) {
                QJsonObject choice = doc.object()["choices"].toArray().at(0).toObject();
                QJsonObject message = choice["message"].toObject();
                if (message.contains("content")) {
                    QString response = message["content"].toString();
                    
                    // 将API返回的内容显示在文本展示框中（覆盖原来的文字）
                    instructionText->setText(response);
                    
                    // 如果是第一次提问的回答，更新输入框提示文本
                    if (!isFirstQuestion) {
                        inputField->setPlaceholderText("请在此输入您的基本信息...");
                    } else {
                        // 重置为初始状态
                        inputField->setPlaceholderText("请在此输入您的问题...");
                        
                        // 如果已经生成了解决方案，显示生成计划表按钮
                        if (hasGeneratedSolution) {
                            generateScheduleBtn->show();
                        }
                    }
                } else {
                    instructionText->setText("抱歉，无法获取有效的回复内容。请重试。");
                }
            } else {
                instructionText->setText("抱歉，服务器返回的数据格式有误。请重试。");
            }
        } else {
            instructionText->setText("抱歉，请求失败：" + reply->errorString() + "。请重试。");
        }
        
        reply->deleteLater();
    });
}

// 添加生成计划表的方法
void CustomizePage::onGenerateScheduleClicked()
{
    // 禁用按钮，防止重复点击
    generateScheduleBtn->setEnabled(false);
    
    // 构建请求提示
    QString prompt = QString("我遇到了%1的问题，现在想要你帮我解决，但因为不同的人需要不同的有针对性的方针，"
                           "以下是我的基本信息：%2。请你根据我的基本信息来为我制定一张计划表，"
                           "表上的内容是我为了解决我的问题每日需要完成的任务")
                           .arg(originalQuestion, userInfo);
    
    // 构建API请求
    QJsonObject json{
        {"model", "deepseek-chat"},
        {"temperature", 0.7},
        {"max_tokens", 2048},
    };
    
    // 构建消息数组
    QJsonArray messages = {
        QJsonObject{
            {"role", "system"}, 
            {"content", "你是一个专业的问题解决顾问，擅长针对不同用户的需求提供个性化的解决方案和计划表。"}
        },
        QJsonObject{
            {"role", "user"}, 
            {"content", prompt}
        }
    };
    
    json["messages"] = messages;
    
    // 创建网络请求
    QNetworkRequest request(QUrl("https://api.deepseek.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());
    
    // 发送请求
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    
    // 处理响应
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // 重新启用按钮
        generateScheduleBtn->setEnabled(true);
        
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isObject() && doc.object().contains("choices")) {
                QJsonObject choice = doc.object()["choices"].toArray().at(0).toObject();
                QJsonObject message = choice["message"].toObject();
                if (message.contains("content")) {
                    QString response = message["content"].toString();
                    
                    // 将计划表追加到现有内容后面
                    QString currentText = instructionText->toPlainText();
                    instructionText->setText(currentText + "\n\n--- 每日计划表 ---\n\n" + response);
                    
                    // 滚动到底部显示新内容
                    instructionText->verticalScrollBar()->setValue(
                        instructionText->verticalScrollBar()->maximum());
                } else {
                    instructionText->setText(instructionText->toPlainText() + 
                                           "\n\n抱歉，无法获取有效的计划表内容。请重试。");
                }
            } else {
                instructionText->setText(instructionText->toPlainText() + 
                                       "\n\n抱歉，服务器返回的数据格式有误。请重试。");
            }
        } else {
            instructionText->setText(instructionText->toPlainText() + 
                                   "\n\n抱歉，请求失败：" + reply->errorString() + "。请重试。");
        }
        
        reply->deleteLater();
    });
}
