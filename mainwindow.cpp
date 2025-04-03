#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_sizeAnimation(new QPropertyAnimation(this, "geometry"))
    , titleBar(new QWidget(this))
    , titleLabel(nullptr)
{
    userGender = "未选择";
    ui->setupUi(this);

    // 设置窗口为无边框
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 初始化网络管理器
    networkManager = new QNetworkAccessManager(this);
    apiKey = "sk-eea6568b51c74da88e91f32f91485ab9"; // 替换为你的API密钥

    // 连接发送按钮点击事件
    connect(ui->pushButtonSend, &QPushButton::clicked, this, &MainWindow::on_pushButtonSend_clicked);

    // 创建自定义标题栏
    titleBar->setFixedHeight(40);
    titleBar->setStyleSheet("background-color: #F8F9FA; border-top-left-radius: 8px; border-top-right-radius: 8px;");
    titleBar->setParent(this); // 显式设置父对象

    // 标题栏布局
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(20, 0, 20, 0);

    // 标题文本
    titleLabel = new QLabel("智能聊天", this);
    titleLabel->setParent(titleBar);
    titleLabel->setFixedHeight(40);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet("color: #212529; font-size: 20px; font-weight: bold; padding-left: 10px;");

    // 关闭按钮
    QPushButton *closeButton = new QPushButton("×", titleBar);
    closeButton->setFixedSize(30, 30);
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::close);

    closeButton->setStyleSheet(R"(
        QPushButton {
            color: #606060;
            font-size: 18px;
            background: transparent;
            padding: 5px;
            margin-right: 10px;
        }
        QPushButton:hover {
            color: red;
            background-color: rgba(255,0,0,0.1);
        }
        QPushButton:pressed {
            color: darkred;
        }
    )");

    // 添加到标题栏布局
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(closeButton);

    // 将标题栏插入到主布局顶部
    QWidget *central = ui->centralwidget;
    QVBoxLayout *mainLayout = static_cast<QVBoxLayout*>(central->layout());
    mainLayout->insertWidget(0, titleBar);

    // 调整输入框和按钮的DPI适配
    ui->lineEditInput->setMinimumSize(300 * m_scale, 60 * m_scale);
    ui->pushButtonSend->setFixedSize(60 * m_scale, 60 * m_scale);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setUserGender(const QString &gender) {
    userGender = gender;
    setWindowTitle("智能聊天 - 用户性别：" + gender);
    titleLabel->setText("智能聊天 - 用户性别：" + gender); // 直接更新标题
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    const QPointF pos = event->position();
#else
    const QPoint pos = event->pos();
#endif

    const int yPos = pos.y();
    const int titleHeight = titleBar->height();

    if (yPos < titleHeight) {
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
#else
        dragPos = event->globalPos() - frameGeometry().topLeft();
#endif
        m_bDrag = true;
        event->accept();
    } else {
        QMainWindow::mousePressEvent(event);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (m_bDrag) {
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        move(event->globalPosition().toPoint() - dragPos);
#else
        move(event->globalPos() - dragPos);
#endif
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    m_bDrag = false;
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::showEvent(QShowEvent *event) {
    QScreen *screen = this->screen();
    if (screen) {
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        m_scale = screen->devicePixelRatio();
#else
        qreal dpiX = screen->logicalDotsPerInchX();
        m_scale = dpiX / 96.0;
#endif

        const int desiredWidth = 800;
        const int desiredHeight = 600;
        const int scaledWidth = desiredWidth * m_scale;
        const int scaledHeight = desiredHeight * m_scale;

        setFixedSize(scaledWidth, scaledHeight); // 固定尺寸并适配DPI

        // 动画配置
        m_sizeAnimation->setDuration(800);
        m_sizeAnimation->setEasingCurve(QEasingCurve::OutQuint);

        // 起始位置（底部中央）
        QRect screenRect = screen->availableGeometry();
        int startX = screenRect.x() + (screenRect.width() - scaledWidth) / 2;
        int startY = screenRect.bottom();
        m_sizeAnimation->setStartValue(QRect(startX, startY, scaledWidth, 0));

        // 结束位置（居中）
        int endY = screenRect.y() + (screenRect.height() - scaledHeight) / 2;
        m_sizeAnimation->setEndValue(QRect(startX, endY, scaledWidth, scaledHeight));

        // 初始位置
        move(startX, startY);
        setGeometry(0, 0, scaledWidth, 0);

        m_sizeAnimation->start();
    }

    QMainWindow::showEvent(event);
}

void MainWindow::on_pushButtonSend_clicked() {
    QString userInput = ui->lineEditInput->text();
    if (userInput.isEmpty()) return;

    // 显示用户输入
    ui->textEditChat->append("You: " + userInput);
    ui->lineEditInput->clear();

    // 构造符合DeepSeek API要求的请求体
    QUrl url("https://api.deepseek.com/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    QJsonObject json;
    json["model"] = "deepseek-chat";
    json["temperature"] = 0.5; // 添加必要参数（根据API文档调整）
    json["max_tokens"] = 2048;  // 添加必要参数

    QJsonArray messagesArray;
    QJsonObject messageObject;
    messageObject["role"] = "user";
    messageObject["content"] = userInput;
    messagesArray.append(messageObject);
    json["messages"] = messagesArray;

    QJsonDocument doc(json);
    qDebug() << "发送的请求数据：" << doc.toJson(); // 调试输出

    // 发送POST请求
    networkManager->post(request, doc.toJson());
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onReplyFinished);
}

void MainWindow::onReplyFinished(QNetworkReply *reply) {
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    QByteArray responseContent = reply->readAll();

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseContent);
        if (jsonResponse.isNull()) {
            ui->textEditChat->append("Error: 无效的JSON响应");
            return;
        }

        QJsonObject jsonObject = jsonResponse.object();
        if (jsonObject.contains("choices")) {
            QJsonArray choicesArray = jsonObject["choices"].toArray();
            if (!choicesArray.isEmpty()) {
                QJsonObject firstChoice = choicesArray[0].toObject();
                QString botReply = firstChoice["message"].toObject()["content"].toString();
                ui->textEditChat->append("Bot: " + botReply);
            }
        } else {
            // 备用解析逻辑：直接显示响应内容
            ui->textEditChat->append("Bot响应：");
            ui->textEditChat->append(QString(responseContent));
        }
    } else {
        // 显示详细错误信息
        QString errorMessage = QString("错误 %1: %2\n服务器响应：%3")
                                   .arg(statusCode.toInt())
                                   .arg(reply->errorString())
                                   .arg(QString(responseContent));
        ui->textEditChat->append(errorMessage);
    }

    reply->deleteLater();
}
