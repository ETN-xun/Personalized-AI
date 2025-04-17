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
    m_rotationAngle(0.0),
    isLoading(false),
    m_scale(1.0)
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
    connect(ui->lineEditInput, &QLineEdit::returnPressed, ui->pushButtonSend, &QPushButton::click);

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

        painter.setPen(QPen(QColor("#4A90E2"), lineWidth));
        painter.drawEllipse(0, 0, indicatorSize, indicatorSize);

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
}

MainWindow::~MainWindow() {
    delete ui;
    delete networkManager; // 补全网络管理器的释放
    delete rotationAnimation;
    delete m_sizeAnimation;
    delete titleBar; // 确保所有子组件都被正确删除
    delete titleLabel;
    delete closeBtn;
    delete minBtn;
    delete maxBtn;
    delete loadIndicator;
}

// 新增：封装发送请求逻辑
void MainWindow::sendChatRequest(const QString &question, bool isOptimization) {
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
    reply->setProperty("requestType", isOptimization ? "optimization" : "chat");
    reply->setProperty("originalInput", question);
}

void MainWindow::on_pushButtonSend_clicked() {
    QString userInput = ui->lineEditInput->text().trimmed();
    if (userInput.isEmpty()) return;

    ui->textEditChat->append(tr("You: %1").arg(userInput));
    ui->lineEditInput->clear();

    // 启动加载动画
    isLoading = true;
    rotationAnimation->start();

    // 先发送优化请求
    sendChatRequest(userInput, true);
}

void MainWindow::onReplyFinished(QNetworkReply *reply) {
    bool isOptimization = reply->property("requestType").toString() == "optimization";
    QString originalInput = reply->property("originalInput").toString();

    if (isOptimization) {
        // 处理优化请求回复
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isObject() && doc.object().contains("choices")) {
                QJsonObject choice = doc.object()["choices"].toArray().at(0).toObject();
                QJsonObject message = choice["message"].toObject();
                if (message.contains("content")) {
                    QString optimizedQuestion = message["content"].toString();
                    // 发送优化后的问题
                    sendChatRequest(optimizedQuestion, false);
                    ui->textEditChat->append(tr("优化后的问题: %1").arg(optimizedQuestion));
                } else {
                    // 优化失败则直接发送原问题
                    sendChatRequest(originalInput, false);
                }
            } else {
                // 结构解析失败则发送原问题
                sendChatRequest(originalInput, false);
            }
        } else {
            // 网络错误处理
            ui->textEditChat->append(tr("优化请求失败，错误代码：%1").arg(reply->error()));
            sendChatRequest(originalInput, false);
        }
    } else {
        // 处理普通聊天回复
        isLoading = false;
        rotationAnimation->stop();
        loadIndicator->clear();
        setRotationAngle(0.0);

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isObject() && doc.object().contains("choices")) {
                QJsonObject choice = doc.object()["choices"].toArray().at(0).toObject();
                QJsonObject message = choice["message"].toObject();
                if (message.contains("content")) {
                    ui->textEditChat->append(tr("Bot: %1").arg(message["content"].toString()));
                } else {
                    ui->textEditChat->append(tr("无效响应：未找到内容字段"));
                }
            } else {
                ui->textEditChat->append(tr("无效响应：缺少choices数组"));
            }
        } else {
            ui->textEditChat->append(tr("Error: %1").arg(reply->errorString()));
        }
    }

    reply->deleteLater();
    update();
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QRect screenRect = screen->availableGeometry();
    QSize initSize(600, 400);

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
}

void MainWindow::toggleMaximize() {
    if (isMaximized()) {
        showNormal();
        maxBtn->setText("□"); // 恢复为方框
    } else {
        showMaximized();
        maxBtn->setText("▢"); // 最大化时显示实心方框
    }
    setWindowTitle(tr("智能聊天 - 用户性别：%1").arg(userGender));
}

void MainWindow::setUserGender(const QString &gender) {
    userGender = gender;
    titleLabel->setText(tr("智能聊天 - 用户性别：%1").arg(gender));
    setWindowTitle(titleLabel->text());
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QMainWindow::paintEvent(event);

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
    QPoint pos;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    pos = event->globalPos();
#else
    pos = event->globalPosition().toPoint();
#endif

    // 使用position()替代y()
    int y = event->position().y();
    if (y < titleBar->height()) {
        dragPos = pos - frameGeometry().topLeft();
        m_bDrag = true;
        event->accept();
        return;
    }

    mousePressRegion = static_cast<Direction>(getMouseRegion(event->pos()));
    if (mousePressRegion == NONE) return;

    resizeStartPos = pos;
    resizeStartGeom = geometry();
    event->accept();
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
}
