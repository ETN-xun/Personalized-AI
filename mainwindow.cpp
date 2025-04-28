#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QWindow>
#include <QHBoxLayout>
#include <QApplication>
#include <QTimer>  // 添加QTimer头文件
#include <QStandardPaths>
#include <QFile>
#include <QRandomGenerator>
#include <QEventLoop>
#include <QDir>
#include <QDateTime>
#include <QListWidget>
#include "customizepage.h"
#include <QCoreApplication>
#include "markdownparser.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    networkManager(new QNetworkAccessManager(this)), // 确保正确初始化顺序
    apiKey("sk-eea6568b51c74da88e91f32f91485ab9"), // 补全初始化顺序
    userGender("未选择"), // 补全初始化顺序
    titleBar(new QWidget(this)),
    titleLabel(new QLabel(titleBar)),
    rotationAnimation(new QPropertyAnimation(this, "rotationAngle")),
    m_sizeAnimation(new QPropertyAnimation(this, "geometry")),
    loadIndicator(new QLabel(titleBar)),
    minBtn(nullptr),
    maxBtn(nullptr),
    closeBtn(nullptr),
    portraitBtn(nullptr), // 初始化画像按钮指针
    portraitWindow(nullptr), // 初始化画像窗口指针
    m_rotationAngle(0.0),
    isLoading(false),
    m_scale(1.0),
    buttonsShown(false),
    hasAskedQuestion(false),
    customizeBtn(nullptr), // 添加量身定制按钮指针
    sideBar(nullptr),
    newChatBtn(nullptr),
    chatHistoryList(nullptr),
    currentChatId("")
{
    // 调整初始化顺序以匹配.h中的成员变量声明顺序
    ui->setupUi(this);

    // 首次计算屏幕缩放比例
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        m_scale = screen->devicePixelRatio();
#else
        qreal dpiX = screen->logicalDotsPerInchX();
        m_scale = dpiX / 96.0;
#endif
    }

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

    titleBar->setFixedHeight(40 * m_scale);
    titleBar->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #4A90E2,stop:1 #63B8FF);"
        "border-top-left-radius:8px;"
        "border-top-right-radius:8px;"
        );

    // 标题栏布局
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(12 * m_scale, 0, 12 * m_scale, 0);

    // 标题和加载指示器
    titleLabel->setStyleSheet("color:white; font:bold 14px 'Microsoft YaHei';");
    titleLayout->addWidget(titleLabel);

    // 加载指示器设置
    loadIndicator->setFixedSize(18 * m_scale, 18 * m_scale);
    loadIndicator->setAlignment(Qt::AlignVCenter);
    loadIndicator->setStyleSheet("background: transparent;");
    loadIndicator->clear();
    titleLayout->addWidget(loadIndicator); // 追加到标题右侧

    // 按钮分隔与右对齐
    titleLayout->addSpacing(15 * m_scale); // 分隔指示器和按钮
    titleLayout->addStretch(); // 伸展让按钮右对齐

    // 窗口控制按钮
    minBtn = new QPushButton("-", titleBar); // 最小化符号更改为"-"
    maxBtn = new QPushButton("□", titleBar); // 初始符号为方框
    closeBtn = new QPushButton("×", titleBar);

    // 统一按钮样式
    auto buttonStyle = QString(
        "QPushButton {"
        "   color: white;"
        "   font-size: 16px;"
        "   border-radius: 11px;"
        "   background: transparent;"
        "}"
        "QPushButton:hover { background: rgba(255,255,255,30); }"
        "QPushButton:pressed { background: rgba(255,255,255,60); }"
        );
    minBtn->setFixedSize(22 * m_scale, 22 * m_scale);
    maxBtn->setFixedSize(22 * m_scale, 22 * m_scale);
    closeBtn->setFixedSize(22 * m_scale, 22 * m_scale);
    minBtn->setStyleSheet(buttonStyle);
    maxBtn->setStyleSheet(buttonStyle);
    closeBtn->setStyleSheet(buttonStyle);

    // 添加按钮到标题栏布局
    titleLayout->addWidget(minBtn);
    titleLayout->addWidget(maxBtn);
    titleLayout->addWidget(closeBtn);
    titleLayout->setAlignment(minBtn, Qt::AlignVCenter);
    titleLayout->setAlignment(maxBtn, Qt::AlignVCenter);
    titleLayout->setAlignment(closeBtn, Qt::AlignVCenter);

    // 主布局设置
    QWidget *central = ui->centralwidget;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(central->layout());
    mainLayout->insertWidget(0, titleBar);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0); // 消除中间区域与输入栏的空白

    // 连接按钮信号
    connect(minBtn, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    connect(maxBtn, &QPushButton::clicked, this, &MainWindow::toggleMaximize);
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    // 输入框和发送按钮样式
    ui->lineEditInput->setStyleSheet(R"(
        QLineEdit {
            background: white;
            border: 2px solid #4A90E2;
            border-radius: 14px;
            padding: 8px 15px;
            font-size: 14px;
        }
        QLineEdit:focus { border-color: #63B8FF; }
    )");

    ui->pushButtonSend->setStyleSheet(R"(
        QPushButton {
            background: #4A90E2;
            color: white;
            border-radius: 15px;
        }
        QPushButton:hover { background: #63B8FF; }
        QPushButton:pressed { background: #3A7BFF; }
    )");
    
    // 修改"量身定制"按钮位置
    customizeBtn = new QPushButton("量身定制", this);
    customizeBtn->setFixedSize(100, 35);
    customizeBtn->setStyleSheet(R"(
        QPushButton {
            background: #4A90E2;
            color: white;
            border-radius: 10px;
            font-size: 14px;
        }
        QPushButton:hover { background: #63B8FF; }
        QPushButton:pressed { background: #3A7BFF; }
    )");

    // 将量身定制按钮添加到标题栏
    if (titleBar->layout()) {
        // 在最小化按钮之前添加量身定制按钮
        int minBtnIndex = titleLayout->indexOf(minBtn);
        titleLayout->insertWidget(minBtnIndex, customizeBtn);
        titleLayout->insertSpacing(minBtnIndex + 1, 10); // 添加一些间距
    }

    // 连接量身定制按钮的点击信号
    connect(customizeBtn, &QPushButton::clicked, this, &MainWindow::openCustomizePage);
    
    connect(ui->lineEditInput, &QLineEdit::returnPressed, this, [this]() {
        // 设置标志，表示用户已经提过问题
        hasAskedQuestion = true;
        
        // 隐藏所有问题按钮
        for (QPushButton* btn : questionButtons) {
            if (btn) {
                btn->hide();
            }
        }
        buttonsShown = true; // 标记为已显示过，防止再次创建
        
        // 触发发送按钮点击
        ui->pushButtonSend->click();
    });

    // 加载指示器动画连接
    connect(rotationAnimation, &QVariantAnimation::valueChanged, [this]() {
        if (!isLoading) {
            loadIndicator->clear();
            return;
        }

        QPixmap pixmap(18 * m_scale, 18 * m_scale);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        const int indicatorSize = 18 * static_cast<int>(m_scale);
        const int lineWidth = 2;

        // 修改第 162 行
        painter.setPen(QPen(QColor("#4A90E2"), lineWidth));
        painter.drawEllipse(0, 0, indicatorSize, indicatorSize);

        // 修改第 165 行
        painter.setPen(QPen(QColor("#FFD700"), lineWidth));
        painter.drawArc(0, 0, indicatorSize, indicatorSize,
                        static_cast<int>(rotationAngle() * 16),
                        -270 * 16);

        loadIndicator->setPixmap(pixmap);
    });

    // 初始化动画参数
    rotationAnimation->setDuration(1000);
    rotationAnimation->setStartValue(0);
    rotationAnimation->setEndValue(360);
    rotationAnimation->setLoopCount(-1);
    rotationAnimation->setEasingCurve(QEasingCurve::Linear);

    // 连接网络请求完成信号到槽
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onReplyFinished);
    
    // 设置窗口标题
    titleLabel->setText("个性化AI助手");
    
    // 创建侧边栏
    sideBar = new QWidget(this);
    sideBar->setFixedWidth(200);
    sideBar->setStyleSheet("background-color: #f0f0f0; border-right: 1px solid #ddd;");
    
    QVBoxLayout *sideBarLayout = new QVBoxLayout(sideBar);
    sideBarLayout->setContentsMargins(10, 10, 10, 10);
    sideBarLayout->setSpacing(10);
    
    // 创建新会话按钮
    newChatBtn = new QPushButton("新会话", sideBar);
    newChatBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4A90E2;
            color: white;
            border-radius: 5px;
            padding: 8px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #63B8FF;
        }
        QPushButton:pressed {
            background-color: #3A7BFF;
        }
    )");
    connect(newChatBtn, &QPushButton::clicked, this, &MainWindow::createNewChat);
    
    // 创建会话历史列表
    chatHistoryList = new QListWidget(sideBar);
    chatHistoryList->setStyleSheet(R"(
        QListWidget {
            background-color: #f8f8f8;
            border-radius: 5px;
            border: 1px solid #ddd;
        }
        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #eee;
        }
        QListWidget::item:selected {
            background-color: #e0e0e0;
            color: #333;
        }
        QListWidget::item:hover {
            background-color: #f0f0f0;
        }
    )");
    connect(chatHistoryList, &QListWidget::currentRowChanged, this, &MainWindow::loadChatHistory);
    
    sideBarLayout->addWidget(newChatBtn);
    sideBarLayout->addWidget(chatHistoryList);
    
    // 调整主窗口布局以适应侧边栏
    QHBoxLayout *sidebarLayout = new QHBoxLayout();
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);
    
    sidebarLayout->addWidget(sideBar);
    
    // 创建中央内容区域容器
    QWidget *contentWidget = new QWidget();
    contentWidget->setLayout(ui->centralwidget->layout());
    ui->centralwidget->setLayout(nullptr);
    
    sidebarLayout->addWidget(contentWidget);
    
    QWidget *container = new QWidget();
    container->setLayout(sidebarLayout);
    setCentralWidget(container);
    
    // 加载聊天历史
    loadChatHistories();
    
    // 创建新会话
    createNewChat();
}

MainWindow::~MainWindow() {
    delete ui;
    delete networkManager;
    delete rotationAnimation;
    delete m_sizeAnimation;
    delete titleBar;
    delete titleLabel;
    delete closeBtn;
    delete minBtn;
    delete maxBtn;
    delete loadIndicator;
    delete portraitBtn; // 释放画像按钮
    delete portraitWindow; // 释放画像窗口
    delete sideBar;
    delete newChatBtn;
    delete chatHistoryList;
    
    // 清理问题按钮
    for (QPushButton* btn : questionButtons) {
        delete btn;
    }
}

// 新增：封装发送请求逻辑
void MainWindow::sendChatRequest(const QString &question, bool isOptimization) {
    requestChatId = currentChatId;
    // 设置加载状态
    isLoading = true;
    rotationAnimation->start();

    // 设置标志，防止再次显示问题按钮
    buttonsShown = true;
    hasAskedQuestion = true;  // 确保记录用户已提问
    
    // 隐藏所有问题按钮
    for (QPushButton* btn : questionButtons) {
        if (btn) {
            btn->hide();
        }
    }
    
    QJsonObject json{
                     {"model", "deepseek-chat"},
                     {"temperature", 0.5},
                     {"max_tokens", 2048},
                     };

    QJsonArray messages;

    if (isOptimization) {
        // 构建优化请求的messages数组
        messages = {
            QJsonObject{{"role", "system"}, {"content", "请优化用户的问题，使其更明确且结构化，同时保持原意。并且你在输出的时候只输出问题即可，不要输出别的。"}},
            QJsonObject{{"role", "user"}, {"content", question}}
        };
    } else {
        // 构建正常聊天请求的messages数组
        messages = {
            QJsonObject{{"role", "user"}, {"content", question}}
        };
    }

    json["messages"] = messages; // 正确赋值给QJsonObject

    QNetworkRequest request(QUrl("https://api.deepseek.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8().data()); // 使用data()获取char*

    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // 停止加载动画
        /*
        isLoading = false;
        rotationAnimation->stop();
        loadIndicator->clear();
        */
        // ... existing code ...
    });
    reply->setProperty("requestType", isOptimization ? "optimization" : "chat");
    reply->setProperty("originalInput", question);
}

void MainWindow::on_pushButtonSend_clicked() {
    QString message = ui->lineEditInput->text().trimmed();
    if (message.isEmpty()) return;
    
    // 设置加载状态
    isLoading = true;
    rotationAnimation->start();
    
    // 设置标志，表示用户已经提过问题
    hasAskedQuestion = true;

    // 隐藏所有问题按钮并设置标志，防止再次显示
    for (QPushButton* btn : questionButtons) {
        if (btn) {
            btn->hide();
        }
    }
    buttonsShown = true; // 标记为已显示过，防止再次创建

    // 清空输入框并添加用户消息到聊天窗口
    ui->lineEditInput->clear();
    ui->textEditChat->append("<div style='text-align:right;'><span style='background-color:#DCF8C6;padding:5px;border-radius:5px;'>" + message + "</span></div>");
    
    // 记录用户消息到当前会话
    QJsonObject chatMessage;
    chatMessage["role"] = "user";
    chatMessage["content"] = message;
    
    // 查找当前会话
    bool isCustomizeSession = false;
    int customizeStep = 0;
    QString originalQuestion;
    
    for (int i = 0; i < chatHistories.size(); i++) {
        if (chatHistories[i]["id"].toString() == currentChatId) {
            QJsonArray messages = chatHistories[i]["messages"].toArray();
            messages.append(chatMessage);
            chatHistories[i]["messages"] = messages;
            
            // 检查是否是量身定制会话
            if (chatHistories[i]["isCustomizeSession"].toBool()) {
                isCustomizeSession = true;
                customizeStep = chatHistories[i]["customizeStep"].toInt();
                
                // 如果是第一步，保存原始问题
                if (customizeStep == 1) {
                    chatHistories[i]["originalQuestion"] = message;
                    chatHistories[i]["customizeStep"] = 2; // 更新为第二步
                    originalQuestion = message;
                } else {
                    originalQuestion = chatHistories[i]["originalQuestion"].toString();
                }
            }
            
            // 更新会话标题（使用用户的第一条消息作为标题）
            if (messages.size() == 1 && !isCustomizeSession) {
                QString title = message;
                if (title.length() > 20) {
                    title = title.left(20) + "...";
                }
                chatHistories[i]["title"] = title;
                chatHistoryList->item(i)->setText(title);
            }
            break;
        }
    }
    
    // 保存聊天历史
    saveChatHistory();

    // 发送聊天请求
    if (isCustomizeSession) {
        if (customizeStep == 1) {
            // 第一步：用户刚输入问题，AI需要询问基本信息
            QString prompt = QString("我遇到了%1的问题，现在想要你帮我解决，但因为不同的人需要不同的有针对性的方针，"
                                   "所以我希望你能向我提几个问题来收集解决这个问题所需的基本信息，"
                                   "来为我制定个性化的解决方案。你需要将问题一次性全部问出").arg(message);
            
            sendCustomizedChatRequest(prompt);
        } else if (customizeStep == 2) {
            // 第二步：用户已提供基本信息，生成解决方案
            QString prompt = QString("我遇到了%1的问题，现在想要你帮我解决，但因为不同的人需要不同的有针对性的方针，"
                                   "以下是我的基本信息：%2。请你根据我的基本信息来为我制定一套个性化方案")
                                   .arg(originalQuestion, message);
            
            sendCustomizedChatRequest(prompt);
            
            // 更新会话，添加生成计划表的选项
            for (int i = 0; i < chatHistories.size(); i++) {
                if (chatHistories[i]["id"].toString() == currentChatId) {
                    chatHistories[i]["userInfo"] = message;
                    chatHistories[i]["customizeStep"] = 3; // 更新为第三步（可以生成计划表）
                    break;
                }
            }
            saveChatHistory();
        } else if (customizeStep == 3) {
            // 第三步：用户可能想要生成计划表或继续提问
            QString userInfo;
            
            // 获取用户信息和原始问题
            for (int i = 0; i < chatHistories.size(); i++) {
                if (chatHistories[i]["id"].toString() == currentChatId) {
                    userInfo = chatHistories[i]["userInfo"].toString();
                    originalQuestion = chatHistories[i]["originalQuestion"].toString();
                    break;
                }
            }
            
            // 检查是否是请求生成计划表
            if (message.contains("计划表") || message.contains("日程") || message.contains("安排")) {
                QString prompt = QString("我遇到了%1的问题，现在想要你帮我解决，但因为不同的人需要不同的有针对性的方针，"
                                       "以下是我的基本信息：%2。请你根据我的基本信息来为我制定一张计划表，"
                                       "表上的内容是我为了解决我的问题每日需要完成的任务")
                                       .arg(originalQuestion, userInfo);
                
                sendCustomizedChatRequest(prompt);
            } else {
                // 普通问题，继续使用个性化上下文
                sendChatRequest(message, false);
            }
        }
    } else {
        sendChatRequest(message, false);
    }
}

void MainWindow::onReplyFinished(QNetworkReply *reply) {
    // 停止加载动画 
    isLoading = false; 
    rotationAnimation->stop(); 
    loadIndicator->clear(); 
    
    // 使用发送请求时的会话ID而不是当前会话ID
    QString chatId = requestChatId;
    
    // 检查请求类型 
    QString requestType = reply->property("requestType").toString(); 
    
    // 处理量身定制响应 
    if (requestType == "customized") { 
        if (reply->error() == QNetworkReply::NoError) { 
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll()); 
            if (doc.isObject() && doc.object().contains("choices")) { 
                QJsonObject choice = doc.object()["choices"].toArray().at(0).toObject(); 
                QJsonObject message = choice["message"].toObject(); 
                if (message.contains("content")) { 
                    QString response = message["content"].toString(); 

                    // 使用MarkdownParser处理AI回复内容 
                    MarkdownParser parser;
                    QString htmlContent = parser.toHtml(response);
                    
                    // 查找对应的聊天历史
                    int historyIndex = -1;
                    for (int i = 0; i < chatHistories.size(); ++i) {
                        if (chatHistories[i]["id"].toString() == chatId) {
                            historyIndex = i;
                            break;
                        }
                    }
                    
                    if (historyIndex != -1) {
                        // 更新对应的聊天历史
                        QJsonArray messages = chatHistories[historyIndex]["messages"].toArray();
                        QJsonObject aiMessage;
                        aiMessage["role"] = "assistant";
                        aiMessage["content"] = response;
                        messages.append(aiMessage);
                        chatHistories[historyIndex]["messages"] = messages;
                        saveChatHistory();
                        
                        // 如果当前显示的不是请求发起的会话，则不更新UI
                        if (currentChatId != chatId) {
                            return;
                        }
                        
                        // 添加AI回复到聊天窗口 - 确保左对齐
                        ui->textEditChat->append("<div style='color:#E91E63; font-weight:bold;'>AI助手:</div>"); 
                        ui->textEditChat->append("<div style='margin-left:10px;'>" + htmlContent + "</div>"); 
                    }
                } 
            } 
        } else { 
            // 处理错误情况 
            ui->textEditChat->append("<div style='color:red;'>错误: " + reply->errorString() + "</div>"); 
        } 
    } else if (requestType == "optimization") { 
        // 处理优化响应 
        if (reply->error() == QNetworkReply::NoError) { 
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll()); 
            if (doc.isObject() && doc.object().contains("choices")) { 
                QJsonObject choice = doc.object()["choices"].toArray().at(0).toObject(); 
                QJsonObject message = choice["message"].toObject(); 
                if (message.contains("content")) { 
                    QString optimizedQuestion = message["content"].toString(); 
                    
                    // 发送优化后的问题 
                    sendChatRequest(optimizedQuestion, false); 
                } 
            } 
        } else { 
            // 处理错误情况 
            ui->textEditChat->append("<div style='color:red;'>错误: " + reply->errorString() + "</div>"); 
        } 
    } else { 
        // 处理普通聊天响应 
        if (reply->error() == QNetworkReply::NoError) { 
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll()); 
            if (doc.isObject() && doc.object().contains("choices")) { 
                QJsonObject choice = doc.object()["choices"].toArray().at(0).toObject(); 
                QJsonObject message = choice["message"].toObject(); 
                if (message.contains("content")) { 
                    QString response = message["content"].toString(); 
                    
                    // 使用MarkdownParser处理AI回复内容 
                    MarkdownParser parser;
                    QString htmlContent = parser.toHtml(response);
                    
                    // 查找对应的聊天历史
                    int historyIndex = -1;
                    for (int i = 0; i < chatHistories.size(); ++i) {
                        if (chatHistories[i]["id"].toString() == chatId) {
                            historyIndex = i;
                            break;
                        }
                    }
                    
                    if (historyIndex != -1) {
                        // 更新对应的聊天历史
                        QJsonArray messages = chatHistories[historyIndex]["messages"].toArray();
                        QJsonObject aiMessage;
                        aiMessage["role"] = "assistant";
                        aiMessage["content"] = response;
                        messages.append(aiMessage);
                        chatHistories[historyIndex]["messages"] = messages;
                        saveChatHistory();
                        
                        // 如果当前显示的不是请求发起的会话，则不更新UI
                        if (currentChatId != chatId) {
                            return;
                        }
                        
                        // 添加AI回复到聊天窗口 
                        ui->textEditChat->append("<div style='color:#E91E63; font-weight:bold;'>AI助手:</div>"); 
                        ui->textEditChat->append("<div style='margin-left:10px;'>" + htmlContent + "</div>"); 
                    }
                } 
            } 
        } else { 
            // 处理错误情况 
            ui->textEditChat->append("<div style='color:red;'>错误: " + reply->errorString() + "</div>"); 
        } 
    } 
    
    reply->deleteLater(); 
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QRect screenRect = screen->availableGeometry();
    QSize initSize(800, 600); // 增加初始窗口大小以适应侧边栏

    m_sizeAnimation->setStartValue(QRect(
        (screenRect.width() - initSize.width()) / 2,
        (screenRect.height() - initSize.height()) / 2,
        0, 0
        ));
    m_sizeAnimation->setEndValue(QRect(
        (screenRect.width() - initSize.width()) / 2,
        (screenRect.height() - initSize.height()) / 2,
        initSize.width(), initSize.height()
        ));
    m_sizeAnimation->start();
    
    // 显示加载指示器
    isLoading = true;
    rotationAnimation->start();

    // 使用QTimer延迟创建问题按钮，让窗口先显示出来
    QTimer::singleShot(100, this, [this]() {
        if (!hasAskedQuestion && !buttonsShown) {
            createQuestionButtons();
            // 停止加载动画
            isLoading = false;
            rotationAnimation->stop();
            loadIndicator->clear();
        }
    });
}

void MainWindow::toggleMaximize() {
    if (isMaximized()) {
        // 创建动画效果
        QRect normalGeom = normalGeometry();
        QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
        animation->setDuration(300); // 300毫秒的动画
        animation->setStartValue(geometry());
        animation->setEndValue(normalGeom);
        animation->setEasingCurve(QEasingCurve::OutCubic); // 使用平滑的缓动曲线
        
        // 修改连接方式，确保窗口状态正确更新
        connect(animation, &QPropertyAnimation::finished, this, [this]() {
            showNormal();
            setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, 
                QSize(800, 600), screen()->availableGeometry()));
            maxBtn->setText("□");
            // 确保窗口状态同步更新
            QCoreApplication::processEvents();
        });
        
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        // 保存当前几何信息
        QRect startGeom = geometry();
        
        // 获取最大化后的几何信息
        QScreen *screen = windowHandle()->screen();
        QRect maxGeom = screen->availableGeometry();
        
        // 创建动画效果
        QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
        animation->setDuration(300); // 300毫秒的动画
        animation->setStartValue(startGeom);
        animation->setEndValue(maxGeom);
        animation->setEasingCurve(QEasingCurve::OutCubic); // 使用平滑的缓动曲线
        
        // 修改连接方式，确保状态正确更新
        connect(animation, &QPropertyAnimation::finished, this, [this]() {
            showMaximized();
            maxBtn->setText("❐");
            // 强制刷新窗口状态
            resize(width(), height());
        });
        
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::setUserGender(const QString &gender) {
    userGender = gender;
    titleLabel->setText(tr("智能聊天 - 用户性别：%1").arg(gender));
    setWindowTitle(titleLabel->text());
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QMainWindow::paintEvent(event);

    // 只在非最大化状态下绘制调整大小的三角形
    if (!isMaximized()) {
        int corner_padding = static_cast<int>(20 * m_scale);
        QPointF points[3];
        points[0] = QPointF(width(), height());
        points[1] = QPointF(width() - corner_padding, height());
        points[2] = QPointF(width(), height() - corner_padding);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QColor handleColor("#4A90E2");
        handleColor.setAlpha(150);
        painter.setBrush(handleColor);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(points, 3);
    }
}

int MainWindow::getMouseRegion(const QPoint &pos) const {
    const int PADDING = static_cast<int>(20 * m_scale);
    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();

    if (y < titleBar->height()) return static_cast<int>(NONE); // 返回整数类型

    if (x < PADDING && y < PADDING) return static_cast<int>(LEFT_TOP);
    if (x >= w - PADDING && y >= h - PADDING) return static_cast<int>(RIGHT_BOTTOM);
    if (x < PADDING && y >= h - PADDING) return static_cast<int>(LEFT_BOTTOM);
    if (x >= w - PADDING && y < PADDING) return static_cast<int>(RIGHT_TOP);

    if (x < PADDING) return static_cast<int>(LEFT);
    if (x >= w - PADDING) return static_cast<int>(RIGHT);
    if (y < PADDING) return static_cast<int>(UP);
    if (y >= h - PADDING) return static_cast<int>(DOWN);

    return static_cast<int>(NONE);
}

void MainWindow::updateCursorShape(const QPoint &pos) {
    int region = getMouseRegion(pos);
    setCursor(Qt::ArrowCursor); // 默认箭头

    switch (region) {
    case static_cast<int>(LEFT_TOP): case static_cast<int>(RIGHT_BOTTOM):
        setCursor(Qt::SizeFDiagCursor);
        break;
    case static_cast<int>(LEFT_BOTTOM): case static_cast<int>(RIGHT_TOP):
        setCursor(Qt::SizeBDiagCursor);
        break;
    case static_cast<int>(LEFT): case static_cast<int>(RIGHT):
        setCursor(Qt::SizeHorCursor);
        break;
    case static_cast<int>(UP): case static_cast<int>(DOWN):
        setCursor(Qt::SizeVerCursor);
        break;
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // 检查是否点击在标题栏上
        if (titleBar->geometry().contains(event->pos())) {
            m_bDrag = true;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            dragPos = event->globalPos() - frameGeometry().topLeft();
#else
            dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
#endif
            event->accept();
            return;
        }
        
        // 检查是否点击在窗口边缘（用于调整大小）
        mousePressRegion = static_cast<Direction>(getMouseRegion(event->pos()));
        if (mousePressRegion != NONE) {
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            resizeStartPos = event->globalPos();
#else
            resizeStartPos = event->globalPosition().toPoint();
#endif
            resizeStartGeom = geometry();
            event->accept();
            return;
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (mousePressRegion != NONE) {
        QPoint delta;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        delta = event->globalPos() - resizeStartPos;
#else
        delta = event->globalPosition().toPoint() - resizeStartPos;
#endif

        QRect newGeom = resizeStartGeom;
        switch (mousePressRegion) {
        case RIGHT_BOTTOM:
            newGeom.setWidth(newGeom.width() + delta.x());
            newGeom.setHeight(newGeom.height() + delta.y());
            break;
        case RIGHT:
            newGeom.setWidth(newGeom.width() + delta.x());
            break;
        case DOWN:
            newGeom.setHeight(newGeom.height() + delta.y());
            break;
        case LEFT_BOTTOM:
            newGeom.setLeft(newGeom.left() + delta.x());
            newGeom.setHeight(newGeom.height() + delta.y());
            break;
        case LEFT_TOP:
            newGeom.setTopLeft(newGeom.topLeft() + delta);
            break;
        case RIGHT_TOP:
            newGeom.setTopRight(newGeom.topRight() + QPoint(newGeom.topRight().x() + delta.x(), newGeom.topRight().y() + delta.y()));
            newGeom.setHeight(newGeom.height() + delta.y());
            break;
        case LEFT:
            newGeom.setLeft(newGeom.left() + delta.x());
            newGeom.setWidth(newGeom.width() - delta.x()); // 左侧缩放需要调整宽度
            break;
        case UP:
            newGeom.setTop(newGeom.top() + delta.y());
            newGeom.setHeight(newGeom.height() - delta.y());
            break;
        }

        // 最小尺寸限制
        if (newGeom.width() < 200) newGeom.setWidth(200);
        if (newGeom.height() < 150) newGeom.setHeight(150);

        setGeometry(newGeom);
    } else {
        updateCursorShape(event->pos());
        if (m_bDrag) {
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            move(event->globalPos() - dragPos);
#else
            move(event->globalPosition().toPoint() - dragPos);
#endif
            event->accept();
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    mousePressRegion = NONE;
    m_bDrag = false;
    setCursor(Qt::ArrowCursor);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    // 双击标题栏切换最大化状态
    if (event->button() == Qt::LeftButton && titleBar->geometry().contains(event->pos())) {
        toggleMaximize();
        event->accept();
        return;
    }
    QMainWindow::mouseDoubleClickEvent(event);
}

// 在resizeEvent函数中添加读取逻辑
// 新增：创建问题按钮
void MainWindow::createQuestionButtons() {
    // 如果用户已经提问，则不创建按钮
    if (hasAskedQuestion) {
        return;
    }
    
    // 重置按钮显示状态
    buttonsShown = true;
    
    // 从hobbies.json读取兴趣及其权重
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QFile file(path + "/hobbies.json");
    QList<QPair<QString, int>> hobbyWeights;
    QStringList zeroWeightHobbies;
    QStringList positiveWeightHobbies;
    QList<int> positiveWeights;
    int totalWeight = 0;
    
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isArray()) {
            QJsonArray array = doc.array();
            for (const QJsonValue &value : array) {
                QJsonObject obj = value.toObject();
                QString hobby = obj["name"].toString();
                int weight = obj["weight"].toInt();
                
                if (weight == 0) {
                    zeroWeightHobbies.append(hobby);
                } else {
                    positiveWeightHobbies.append(hobby);
                    positiveWeights.append(weight);
                    totalWeight += weight;
                }
            }
        }
        file.close();
    }
    
    // 生成三个问题
    QStringList newQuestions;
    QStringList selectedHobbies; // 临时数组存储选择的兴趣
    
    // 读取topics.json文件
    QFile topicsFile(path + "/topics.json");
    QJsonObject topicsData;
    
    if (topicsFile.open(QIODevice::ReadOnly)) {
        QJsonDocument topicsDoc = QJsonDocument::fromJson(topicsFile.readAll());
        if (topicsDoc.isObject()) {
            topicsData = topicsDoc.object();
        }
        topicsFile.close();
    }
    
    for (int i = 0; i < 3; i++) {
        QString selectedHobby;
        
        // 10%概率选择权重为0的兴趣，90%概率按权重选择权重大于0的兴趣
        bool useZeroWeight = (QRandomGenerator::global()->bounded(100) < 10) && !zeroWeightHobbies.isEmpty();
        
        if (useZeroWeight) {
            // 随机选择一个权重为0的兴趣
            int randomIndex = QRandomGenerator::global()->bounded(zeroWeightHobbies.size());
            selectedHobby = zeroWeightHobbies.at(randomIndex);
        } else if (!positiveWeightHobbies.isEmpty()) {
            // 按权重随机选择一个权重大于0的兴趣
            int randomValue = QRandomGenerator::global()->bounded(totalWeight);
            int accumulatedWeight = 0;
            
            for (int j = 0; j < positiveWeightHobbies.size(); j++) {
                accumulatedWeight += positiveWeights.at(j);
                if (randomValue < accumulatedWeight) {
                    selectedHobby = positiveWeightHobbies.at(j);
                    break;
                }
            }
        } else {
            // 如果没有任何兴趣，使用默认问题
            newQuestions << "1+1等于几？" << "2+2等于5吗？" << "3+3小于10吗？";
            break;
        }
        
        // 从topics.json中获取与所选兴趣相关的问题
        if (!selectedHobby.isEmpty()) {
            selectedHobbies.append(selectedHobby); // 将选择的兴趣添加到临时数组
            
            if (topicsData.contains(selectedHobby) && topicsData[selectedHobby].isArray()) {
                QJsonArray hobbyTopics = topicsData[selectedHobby].toArray();
                if (!hobbyTopics.isEmpty()) {
                    // 随机选择一个问题
                    int randomTopicIndex = QRandomGenerator::global()->bounded(hobbyTopics.size());
                    QString question = hobbyTopics[randomTopicIndex].toString();
                    newQuestions.append(question);
                } else {
                    // 如果该兴趣没有问题，使用默认问题
                    newQuestions.append("你对" + selectedHobby + "有什么看法？");
                }
            } else {
                // 如果在topics.json中找不到该兴趣，使用默认问题
                newQuestions.append("你对" + selectedHobby + "有什么看法？");
            }
        }
    }
    
    // 如果成功生成了问题，更新问题列表
    if (!newQuestions.isEmpty()) {
        questions = newQuestions;
    } else if (questions.isEmpty()) {
        // 如果没有生成问题且问题列表为空，使用默认问题
        questions << "1+1等于几？" << "2+2等于5吗？" << "3+3小于10吗？";
    }
    
    // 清理之前可能存在的按钮
    for (QPushButton* btn : questionButtons) {
        if (btn) {
            btn->deleteLater();
        }
    }
    questionButtons.clear();
    
    // 创建新按钮
    for (int i = 0; i < questions.size(); ++i) {
        QPushButton *btn = new QPushButton(questions[i], this);
        
        // 设置按钮样式
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #4A90E2;
                color: white;
                border-radius: 15px;
                padding: 10px 20px;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: #63B8FF;
            }
            QPushButton:pressed {
                background-color: #3A7BFF;
            }
        )");
        
        // 设置按钮位置 - 修改为中央区域左上角垂直排列
        int btnHeight = 50;
        int spacing = 20;
        // 考虑侧边栏宽度，将按钮放在中央区域左上方
        int startX = sideBar->width() + 20; // 从侧边栏右侧开始，加上一些左边距
        int startY = titleBar->height() + 20; // 从标题栏下方开始，加上一些上边距
        
        // 根据文本长度计算按钮宽度
        QFontMetrics fontMetrics(btn->font());
        int textWidth = fontMetrics.horizontalAdvance(questions[i]);
        int btnWidth = textWidth + 80; // 文本宽度加上左右内边距
        
        // 设置最小和最大宽度限制
        btnWidth = qMax(200, btnWidth); // 最小宽度为200
        btnWidth = qMin(600, btnWidth); // 最大宽度从400改为600
        
        btn->setGeometry(
            startX,
            startY + i * (btnHeight + spacing),
            btnWidth,
            btnHeight
        );
        
        // 连接信号槽
        connect(btn, &QPushButton::clicked, this, [this, i, selectedHobbies]() {
            onQuestionButtonClicked();
            // 更新hobbies.json中对应兴趣的权重
            QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QFile file(path + "/hobbies.json");
            if (file.open(QIODevice::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
                if (doc.isArray()) {
                    QJsonArray array = doc.array();
                    for (int j = 0; j < array.size(); ++j) {
                        QJsonObject obj = array[j].toObject();
                        if (obj["name"].toString() == selectedHobbies[i]) {
                            obj["weight"] = obj["weight"].toInt() + 1;
                            array[j] = obj;
                            break;
                        }
                    }
                    file.close();
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(QJsonDocument(array).toJson());
                        file.close();
                    }
                }
            }
        });
        
        if(!hasAskedQuestion)
        {
            btn->show();
        }else{
            btn->hide();
        }
        questionButtons.append(btn);
    }
    // 添加回这三行代码，确保加载动画停止
    /*
    isLoading = false;
    rotationAnimation->stop();
    loadIndicator->clear();
    */
}

// 新增：问题按钮点击处理
void MainWindow::onQuestionButtonClicked() {
    // 获取发送者按钮
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    // 获取问题文本
    QString question = button->text();
    
    // 清空输入框并添加用户消息到聊天窗口
    ui->lineEditInput->clear();
    ui->textEditChat->append("<div style='text-align:right;'><span style='background-color:#DCF8C6;padding:5px;border-radius:5px;'>" + question + "</span></div>");
    
    // 记录用户消息到当前会话
    QJsonObject chatMessage;
    chatMessage["role"] = "user";
    chatMessage["content"] = question;
    
    // 查找当前会话
    for (int i = 0; i < chatHistories.size(); i++) {
        if (chatHistories[i]["id"].toString() == currentChatId) {
            QJsonArray messages = chatHistories[i]["messages"].toArray();
            messages.append(chatMessage);
            chatHistories[i]["messages"] = messages;
            
            // 更新会话标题（使用用户的第一条消息作为标题）
            if (messages.size() == 1) {
                QString title = question;
                if (title.length() > 20) {
                    title = title.left(20) + "...";
                }
                chatHistories[i]["title"] = title;
                
                // 更新UI中的标题
                QListWidgetItem* item = chatHistoryList->item(i);
                if (item) {
                    QWidget* widget = chatHistoryList->itemWidget(item);
                    if (widget) {
                        QLabel* titleLabel = widget->findChild<QLabel*>();
                        if (titleLabel) {
                            titleLabel->setText(title);
                        }
                    }
                }
            }
            break;
        }
    }
    
    // 保存聊天历史
    saveChatHistory();
    
    // 设置加载状态
    isLoading = true;
    rotationAnimation->start();
    
    // 隐藏所有问题按钮
    for (QPushButton* btn : questionButtons) {
        if (btn) {
            btn->hide();
        }
    }
    
    // 设置标志，防止再次显示问题按钮
    buttonsShown = true;
    hasAskedQuestion = true;  // 确保记录用户已提问
    
    // 发送聊天请求
    sendChatRequest(question, false);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    QScreen *screen = windowHandle()->screen();
    if (!screen) return;

    m_scale = screen->devicePixelRatio();

    // 动态调整标题栏和按钮尺寸
    titleBar->setFixedHeight(40 * m_scale);
    loadIndicator->setFixedSize(18 * m_scale, 18 * m_scale);
    minBtn->setFixedSize(22 * m_scale, 22 * m_scale);
    maxBtn->setFixedSize(22 * m_scale, 22 * m_scale);
    closeBtn->setFixedSize(22 * m_scale, 22 * m_scale);

    // 更新标题栏布局
    auto *titleLayout = qobject_cast<QHBoxLayout*>(titleBar->layout());
    if (titleLayout) {
        titleLayout->setContentsMargins(12 * m_scale, 0, 12 * m_scale, 0);
        titleLayout->setSpacing(10 * m_scale);
    }
    
    // 在最大化状态下，确保输入栏位于窗口底部
    if (isMaximized()) {
        QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
        if (mainLayout) {
            mainLayout->setContentsMargins(0, 0, 0, 0);
        }
    }

    // 确保画像按钮只创建一次
    if (!portraitBtn) {
        portraitBtn = new QPushButton("画像", this);
        portraitBtn->setFixedSize(50, 50);
        
        if (titleBar) {
            QHBoxLayout *titleLayout = qobject_cast<QHBoxLayout*>(titleBar->layout());
            if (titleLayout) {
                int minBtnIndex = titleLayout->indexOf(minBtn);
                titleLayout->insertWidget(minBtnIndex, portraitBtn);
            }
        }
        
        // 修复：创建独立窗口而不是子窗口
        portraitWindow = new QWidget(nullptr);  // 使用nullptr作为父对象
        portraitWindow->setWindowFlags(Qt::Window); // 设置为独立窗口
        portraitWindow->setWindowTitle("用户画像分析");
        portraitWindow->resize(400, 400);
        
        // 添加布局和饼图组件
        QVBoxLayout *layout = new QVBoxLayout(portraitWindow);
        layout->setContentsMargins(20, 20, 20, 20);
        
        // 创建并添加饼图组件
        PieChartWidget *pieChart = new PieChartWidget(portraitWindow);
        layout->addWidget(pieChart);
        
        // 确保窗口初始隐藏
        portraitWindow->hide();
        
        // 连接信号
        // 在portraitBtn的点击事件处理中
        connect(portraitBtn, &QPushButton::clicked, this, [this]() {
        // 添加路径声明
        QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QList<QPair<QString, int>> hobbiesWithWeights;
        QFile file(path + "/hobbies.json");
        
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isArray()) {
                QJsonArray array = doc.array();
                foreach (const QJsonValue &value, array) {
                    QJsonObject obj = value.toObject();
                    QString name = obj["name"].toString();
                    int weight = obj["weight"].toInt();
                    // 只添加权重大于0的兴趣爱好
                    if (weight > 0) {
                        hobbiesWithWeights.append(qMakePair(name, weight));
                    }
                }
            }
            file.close();
        }
        
        // 更新饼图数据
        if (portraitWindow) {
            QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(portraitWindow->layout());
            if (layout && layout->count() > 0) {
                PieChartWidget* pieChart = qobject_cast<PieChartWidget*>(layout->itemAt(0)->widget());
                if (pieChart) {
                    pieChart->setHobbiesWithWeights(hobbiesWithWeights);
                    pieChart->update(); // 强制重绘
                }
            }
        }
        
        // 显示模态窗口
        if (portraitWindow->isHidden()) {
            portraitWindow->show();
        } else {
            portraitWindow->hide();
        }
        });
    }
}

void MainWindow::setUserHobbies(const QStringList &hobbies) {
    userHobbies = hobbies;

    // 读取hobbies.json文件
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QFile file(path + "/hobbies.json");
    QList<QPair<QString, int>> hobbyWeights;
    
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isArray()) {
            QJsonArray array = doc.array();
            for (const QJsonValue &value : array) {
                QJsonObject obj = value.toObject();
                hobbyWeights.append(qMakePair(
                    obj["name"].toString(),
                    obj["weight"].toInt()
                ));
            }
        }
        file.close();
    }

    // 更新饼图数据...
    if (portraitWindow) {
        QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(portraitWindow->layout());
        if (layout) {
            PieChartWidget *pieChart = qobject_cast<PieChartWidget*>(layout->itemAt(0)->widget());
            if (pieChart) {
                QList<QPair<QString, int>> hobbiesWithWeights;
                foreach (const QString &hobby, hobbies) {
                    // Find matching hobby in the loaded weights
                    auto it = std::find_if(hobbyWeights.begin(), hobbyWeights.end(),
                        [&hobby](const QPair<QString, int>& item) { return item.first == hobby; });
                    int weight = (it != hobbyWeights.end()) ? it->second : 1;
                    // 只添加权重大于0的兴趣爱好
                    if (weight > 0) {
                        hobbiesWithWeights.append(qMakePair(hobby, weight));
                    }
                }
                pieChart->setHobbiesWithWeights(hobbiesWithWeights);
            }
        }
    }
}



void PieChartWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 计算饼图的尺寸和位置
    int w = width();
    int h = height();
    int size = qMin(w, h) - 40; // 留出边距
    QRect pieRect((w - size) / 2, (h - size) / 2, size, size);
    
    // 如果没有数据，绘制空饼图
    if (m_hobbiesWithWeights.isEmpty()) {
        painter.setPen(QPen(Qt::gray, 2));
        painter.setBrush(Qt::lightGray);
        painter.drawEllipse(pieRect);
        return;
    }
    
    // 计算总权重
    int totalWeight = 0;
    for (const auto &pair : m_hobbiesWithWeights) {
        totalWeight += pair.second;
    }
    
    // 绘制饼图
    int startAngle = 0;
    QStringList colorList = {"#FF9999", "#66B3FF", "#99FF99", "#FFCC99", "#FF99CC", "#99CCFF", "#CC99FF", "#FFFF99"};
    
    for (int i = 0; i < m_hobbiesWithWeights.size(); ++i) {
        const auto &pair = m_hobbiesWithWeights.at(i);
        int angle = pair.second * 5760 / totalWeight; // 5760 = 16 * 360
        
        QColor color(colorList.at(i % colorList.size()));
        painter.setBrush(color);
        painter.setPen(Qt::white);
        painter.drawPie(pieRect, startAngle, angle);
        
        // 计算文本位置（在扇形中心位置）
        double middleAngle = startAngle + angle / 2;
        double radians = middleAngle * M_PI / (16 * 180);
        double radius = size / 3; // 调整文本距离圆心的距离
        int x = pieRect.center().x() + radius * cos(radians);
        int y = pieRect.center().y() - radius * sin(radians);
        
        // 设置文本样式
        QFont font = painter.font();
        font.setPointSize(14); // 增大字号，原来可能是较小的值
        painter.setFont(font);
        
        // 测量文本尺寸
        QFontMetrics fm(font);
        QString text = pair.first;
        QRect textRect = fm.boundingRect(text);
        textRect.moveCenter(QPoint(x, y));
        
        // 绘制文本背景（半透明白色圆角矩形）
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 180)); // 半透明白色
        painter.drawRoundedRect(textRect.adjusted(-10, -5, 10, 5), 10, 10); // 调整大小并设置圆角
        
        // 绘制文本
        painter.setPen(Qt::black);
        painter.drawText(textRect, Qt::AlignCenter, text);
        
        startAngle += angle;
    }
}
void MainWindow::openCustomizePage()
{
    // 创建新会话
    createNewChat();
    
    // 设置标志，表示用户已经提过问题
    hasAskedQuestion = true;
    
    // 隐藏所有问题按钮
    for (QPushButton* btn : questionButtons) {
        if (btn) {
            btn->hide();
        }
    }
    buttonsShown = true;
    
    // 添加引导性问题，提示用户输入问题
    QString welcomeMessage = "请输入您当前面临的问题，我将为您提供个性化的解决方案。";
    ui->textEditChat->append("<div style='color:#888888; font-style:italic;'>系统: 已进入量身定制模式，AI将根据您的个人信息提供更加个性化的回答</div>");
    ui->textEditChat->append("<br>");
    ui->textEditChat->append("<div style='color:#E91E63; font-weight:bold;'>AI助手:</div>");
    ui->textEditChat->append("<div style='margin-left:10px;'>" + welcomeMessage + "</div>");
    ui->textEditChat->append("<br>");
    
    // 记录系统消息到当前会话
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "你是一个专业的问题解决顾问，擅长针对不同用户的需求提供个性化的解决方案和计划表。";
    
    QJsonObject aiMessage;
    aiMessage["role"] = "assistant";
    aiMessage["content"] = welcomeMessage;
    
    // 查找当前会话并添加消息
    for (int i = 0; i < chatHistories.size(); i++) {
        if (chatHistories[i]["id"].toString() == currentChatId) {
            QJsonArray messages = chatHistories[i]["messages"].toArray();
            messages.append(systemMessage);
            messages.append(aiMessage);
            chatHistories[i]["messages"] = messages;
            
            // 更新会话标题
            chatHistories[i]["title"] = "量身定制会话";
            chatHistoryList->item(i)->setText("量身定制会话");
            
            // 标记为量身定制会话的第一步
            chatHistories[i]["isCustomizeSession"] = true;
            chatHistories[i]["customizeStep"] = 1;
            break;
        }
    }
    
    // 保存聊天历史
    saveChatHistory();
}

// 新增：创建新会话
void MainWindow::createNewChat()
{
    // 生成唯一ID
    QString chatId = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    
    // 创建新会话对象
    QJsonObject chatHistory;
    chatHistory["id"] = chatId;
    bool isCustomize = QObject::sender() == customizeBtn; // 判断触发源是否是量身定制按钮
    chatHistory["title"] = isCustomize ? "量身定制会话" : "新会话";
    chatHistory["messages"] = QJsonArray();
    
    // 添加到会话列表
    chatHistories.prepend(chatHistory);
    
    // 更新UI
    QListWidgetItem* item = new QListWidgetItem();

    
    // 创建一个包含会话标题和删除按钮的小部件
    QWidget* itemWidget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(itemWidget);
    layout->setContentsMargins(8, 5, 8, 5);
    layout->setSpacing(5);
    
    // 会话标题标签
    QLabel* titleLabel = new QLabel(isCustomize ? "量身定制会话" : "新会话", itemWidget);
    //QLabel* titleLabel = new QLabel("新会话", itemWidget);
    titleLabel->setStyleSheet("background: transparent;");
    
    // 删除按钮
    QPushButton* deleteBtn = new QPushButton("×", itemWidget);
    deleteBtn->setFixedSize(20, 20);
    deleteBtn->setStyleSheet(R"(
        QPushButton {
            color: #888;
            background: transparent;
            border: none;
            font-size: 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            color: red;
        }
    )");
    
    // 连接删除按钮的点击信号
    connect(deleteBtn, &QPushButton::clicked, this, [this, chatId]() {
        // 找到要删除的会话索引
        int indexToDelete = -1;
        
        for (int i = 0; i < chatHistories.size(); i++) {
            if (chatHistories[i]["id"].toString() == chatId) {
                indexToDelete = i;
                break;
            }
        }
        
        if (indexToDelete != -1) {
            // 如果删除的是当前会话，创建一个新会话
            bool isCurrentChat = (currentChatId == chatId);
            
            // 从列表中移除会话
            chatHistories.removeAt(indexToDelete);
            delete chatHistoryList->takeItem(indexToDelete);
            
            // 保存更新后的会话历史
            saveChatHistory();
            
            // 如果删除的是当前会话，创建一个新会话
            if (isCurrentChat) {
                createNewChat();
            }
        }
    });
    
    // 修改布局，让标题占据更多空间
    layout->addWidget(titleLabel, 1); // 标题占据剩余空间
    layout->addWidget(deleteBtn, 0);  // 删除按钮不拉伸
    
    // 设置布局
    itemWidget->setLayout(layout);
    
    // 设置列表项
    item->setSizeHint(QSize(chatHistoryList->width() - 10, 40));
    chatHistoryList->insertItem(0, item);
    chatHistoryList->setItemWidget(item, itemWidget);
    chatHistoryList->setCurrentRow(0);
    
    // 更新当前会话ID
    currentChatId = chatId;
    
    // 清空聊天窗口
    ui->textEditChat->clear();
    
    // 保存会话历史
    saveChatHistory();
    
    // 创建问题按钮
    createQuestionButtons();
}

// 新增：加载历史会话
void MainWindow::loadChatHistory(int index) {
    if (index < 0 || index >= chatHistories.size()) {
        return;
    }
    
    // 清空聊天窗口
    ui->textEditChat->clear();
    
    // 获取选中的会话
    QJsonObject chatHistory = chatHistories[index];
    currentChatId = chatHistory["id"].toString();
    
    // 加载会话消息
    QJsonArray messages = chatHistory["messages"].toArray();
    
    // 检查是否是空白会话
    bool isEmptyChat = messages.isEmpty();
    
    // 根据会话是否为空决定是否显示推荐问题按钮
    if (!isEmptyChat) {
        // 如果是空白会话，隐藏所有问题按钮
        for (QPushButton* btn : questionButtons) {
            if (btn) {
                btn->hide();
            }
        }
        buttonsShown = false; // 重置标志，允许在用户输入后再次创建
        hasAskedQuestion = false; // 重置用户提问标志
    } else {
        // 如果不是空白会话，显示并重新生成推荐问题
        // 先清除现有按钮
        for (QPushButton* btn : questionButtons) {
            if (btn) {
                btn->deleteLater();
            }
        }
        questionButtons.clear();
        buttonsShown = false; // 重置标志，允许重新创建
        
        // 重新生成推荐问题按钮
        createQuestionButtons();
    }
    
    // 显示会话消息
    for (int i = 0; i < messages.size(); i++) {
        QJsonObject message = messages[i].toObject();
        QString role = message["role"].toString();
        QString content = message["content"].toString();
        
        if (role == "user") {
            ui->textEditChat->append("<div style='text-align:right;'><span style='background-color:#DCF8C6;padding:5px;border-radius:5px;'>" + content + "</span></div>");
        } else if (role == "assistant") {
            // 使用MarkdownParser处理AI回复内容
            MarkdownParser parser;
            QString htmlContent = parser.toHtml(content);
            ui->textEditChat->append("<div style='text-align:left;'><span style='background-color:#FFFFFF;padding:5px;border-radius:5px;'>" + htmlContent + "</span></div>");
        }
    }
    
    // 检查是否是量身定制会话
    bool isCustomizeSession = chatHistory["isCustomizeSession"].toBool();
    if (isCustomizeSession) {
        // 更新标题
        titleLabel->setText("个性化AI助手 - 量身定制");
    } else {
        // 更新标题
        titleLabel->setText("个性化AI助手");
    }
}

// 新增：保存聊天历史
void MainWindow::saveChatHistory()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile file(path + "/chat_history.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // 将QList<QJsonObject>转换为QJsonArray
        QJsonArray jsonArray;
        for (const QJsonObject &obj : chatHistories) {
            jsonArray.append(obj);
        }
        
        QJsonDocument doc(jsonArray);
        file.write(doc.toJson());
        file.close();
    }
}

// 新增：加载所有聊天历史
void MainWindow::loadChatHistories()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QFile file(path + "/chat_history.json");
    
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isArray()) {
            QJsonArray array = doc.array();
            for (const QJsonValue &value : array) {
                chatHistories.append(value.toObject());
            }
        }
        file.close();
    }
    
    // 更新UI
    chatHistoryList->clear();
    for (const QJsonObject &chatHistory : chatHistories) {
        // 创建一个包含会话标题和删除按钮的小部件
        QWidget* itemWidget = new QWidget(chatHistoryList);
        QHBoxLayout* layout = new QHBoxLayout(itemWidget);
        layout->setContentsMargins(8, 5, 8, 5);
        layout->setSpacing(5);
        
        // 会话标题标签
        QLabel* titleLabel = new QLabel(chatHistory["title"].toString(), itemWidget);
        titleLabel->setStyleSheet("background: transparent;");
        
        // 删除按钮
        QPushButton* deleteBtn = new QPushButton("×", itemWidget);
        deleteBtn->setFixedSize(20, 20);
        deleteBtn->setStyleSheet(R"(
            QPushButton {
                color: #888;
                background: transparent;
                border: none;
                font-size: 16px;
                font-weight: bold;
            }
            QPushButton:hover {
                color: red;
            }
        )");
        
        // 连接删除按钮的点击信号
        connect(deleteBtn, &QPushButton::clicked, this, [this, chatHistory]() {
            // 找到要删除的会话索引
            QString idToDelete = chatHistory["id"].toString();
            int indexToDelete = -1;
            
            for (int i = 0; i < chatHistories.size(); i++) {
                if (chatHistories[i]["id"].toString() == idToDelete) {
                    indexToDelete = i;
                    break;
                }
            }
            
            if (indexToDelete != -1) {
                // 如果删除的是当前会话，创建一个新会话
                bool isCurrentChat = (currentChatId == idToDelete);
                
                // 从列表中移除会话
                chatHistories.removeAt(indexToDelete);
                delete chatHistoryList->takeItem(indexToDelete);
                
                // 保存更新后的会话历史
                saveChatHistory();
                
                // 如果删除的是当前会话，创建一个新会话
                if (isCurrentChat) {
                    createNewChat();
                }
            }
        });
        
        // 修改布局，让标题占据更多空间
        layout->addWidget(titleLabel, 1); // 标题占据剩余空间
        layout->addWidget(deleteBtn, 0);  // 删除按钮不拉伸
        
        // 设置布局
        itemWidget->setLayout(layout);
        
        // 创建列表项并设置自定义小部件
        QListWidgetItem* item = new QListWidgetItem(chatHistoryList);
        // 设置合适的高度，确保整个内容可见
        item->setSizeHint(QSize(chatHistoryList->width() - 10, 40));
        chatHistoryList->setItemWidget(item, itemWidget);
    }
}

void PieChartWidget::setHobbiesWithWeights(const QList<QPair<QString, int>> &hobbies) {
    m_hobbiesWithWeights = hobbies;
    update(); // 触发重绘
}
void MainWindow::sendCustomizedChatRequest(const QString &prompt) {
    // 设置加载状态
    isLoading = true;
    rotationAnimation->start();
    
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
    
    // 设置请求类型属性，以便在回调中区分
    reply->setProperty("requestType", "customized");
}

