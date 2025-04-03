#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_sizeAnimation(new QPropertyAnimation(this, "geometry")),
    titleBar(new QWidget(this)),
    titleLabel(new QLabel(titleBar)),
    networkManager(new QNetworkAccessManager(this)),
    m_scale(1.0) // 初始化DPI缩放比例
{
    ui->setupUi(this);

    // 设置透明背景和无边框
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

    // 完全移除尺寸限制
    setMinimumSize(0, 0); // 允许最小到0
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); // 最大到系统允许的极限

    // 标题栏初始化
    titleBar->setFixedHeight(40);
    titleBar->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #4A90E2,stop:1 #63B8FF);"
        "border-top-left-radius:8px;"
        "border-top-right-radius:8px;"
        );

    // 标题栏布局
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(12, 0, 12, 0);
    titleLabel->setStyleSheet("color:white; font:bold 16px 'Microsoft Yahei';");
    titleLayout->addWidget(titleLabel);

    // 关闭按钮
    QPushButton *closeBtn = new QPushButton("×", titleBar);
    closeBtn->setFixedSize(30, 30);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   color: white;"
        "   font-size: 20px;"
        "   border-radius: 15px;"
        "   background: transparent;"
        "}"
        "QPushButton:hover {"
        "   background: rgba(255,255,255,30);"
        "}"
        "QPushButton:pressed {"
        "   background: rgba(255,255,255,60);"
        "}"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    titleLayout->addStretch();
    titleLayout->addWidget(closeBtn);

    // 主布局设置
    QWidget *central = ui->centralwidget;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(central->layout());
    mainLayout->insertWidget(0, titleBar);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 输入框和发送按钮样式
    ui->lineEditInput->setStyleSheet(R"(
        QLineEdit{
            background:white;
            border:2px solid #4A90E2;
            border-radius:15px;
            padding:8px 15px;
            font-size:14px;
        }
        QLineEdit:focus{
            border-color:#63B8FF;
        }
    )");

    ui->pushButtonSend->setStyleSheet(R"(
        QPushButton{
            background:#4A90E2;
            color:white;
            border-radius:15px;
        }
        QPushButton:hover{
            background:#63B8FF;
        }
        QPushButton:pressed{
            background:#3A7BFF;
        }
    )");

    connect(ui->lineEditInput, &QLineEdit::returnPressed, ui->pushButtonSend, &QPushButton::click);
}

MainWindow::~MainWindow() {
    delete ui;
}

int MainWindow::getMouseRegion(const QPoint &pos) const {
    const int PADDING = static_cast<int>(20 * m_scale); // 触发区域尺寸随DPI变化
    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();

    // 先判断角落区域
    if (x < PADDING && y < PADDING) return LEFT_TOP;
    if (x >= w - PADDING && y >= h - PADDING) return RIGHT_BOTTOM;
    if (x < PADDING && y >= h - PADDING) return LEFT_BOTTOM;
    if (x >= w - PADDING && y < PADDING) return RIGHT_TOP;

    // 再判断边缘区域
    if (x < PADDING) return LEFT;
    if (x >= w - PADDING) return RIGHT;
    if (y < PADDING) return UP;
    if (y >= h - PADDING) return DOWN;

    return NONE;
}

void MainWindow::updateCursorShape(const QPoint &pos) {
    switch (getMouseRegion(pos)) {
    case LEFT_TOP: case RIGHT_BOTTOM:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case LEFT_BOTTOM: case RIGHT_TOP:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case LEFT: case RIGHT:
        setCursor(Qt::SizeHorCursor);
        break;
    case UP: case DOWN:
        setCursor(Qt::SizeVerCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    QPoint pos;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    pos = event->globalPos();
#else
    pos = event->globalPosition().toPoint();
#endif

    mousePressRegion = static_cast<Direction>(getMouseRegion(event->pos()));
    resizeStartPos = pos;
    resizeStartGeom = geometry();

    // 标题栏拖动
    if (mousePressRegion == NONE && event->pos().y() < titleBar->height()) {
        dragPos = pos - frameGeometry().topLeft();
        m_bDrag = true;
        event->accept();
    }
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

        // 处理窗口大小调整
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
            newGeom.setTopRight(newGeom.topRight() + delta);
            break;
        case LEFT:
            newGeom.setLeft(newGeom.left() + delta.x());
            break;
        case UP:
            newGeom.setTop(newGeom.top() + delta.y());
            break;
        }

        // 更新窗口几何（防止窗口过小）
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

    // 更新DPI缩放比例
    QScreen *screen = windowHandle()->screen();
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    m_scale = screen->devicePixelRatio();
#else
    qreal dpiX = screen->logicalDotsPerInchX();
    m_scale = dpiX / 96.0;
#endif

    // 强制更新paintEvent
    update();
}

void MainWindow::showEvent(QShowEvent *event) {
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->availableGeometry();
    QSize initSize(600, 400);

// 计算DPI比例
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    m_scale = screen->devicePixelRatio();
#else
    qreal dpiX = screen->logicalDotsPerInchX();
    m_scale = dpiX / 96.0;
#endif

    // 窗口弹出动画
    m_sizeAnimation->setDuration(800);
    m_sizeAnimation->setEasingCurve(QEasingCurve::OutQuint);

    QPoint centerPos = screenRect.center() - QPoint(initSize.width()/2, initSize.height()/2);
    m_sizeAnimation->setStartValue(QRect(centerPos.x(), screenRect.bottom(), 0, 0));
    m_sizeAnimation->setEndValue(QRect(centerPos, initSize));
    m_sizeAnimation->start();

    QMainWindow::showEvent(event);
}

void MainWindow::setUserGender(const QString &gender) {
    userGender = gender;
    titleLabel->setText(tr("智能聊天 - 用户性别：%1").arg(gender));
}

// 发送按钮点击事件
void MainWindow::on_pushButtonSend_clicked() {
    QString userInput = ui->lineEditInput->text().trimmed();
    if (userInput.isEmpty()) return;

    ui->textEditChat->append(tr("You: %1").arg(userInput));
    ui->lineEditInput->clear();

    // 创建网络请求
    QUrl url("https://api.deepseek.com/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer sk-eea6568b51c74da88e91f32f91485ab9");

    // 构建JSON参数
    QJsonObject json;
    json["model"] = "deepseek-chat"; // 使用QString隐式转换
    json["temperature"] = 0.5;
    json["max_tokens"] = 2048;

    QJsonArray messagesArray;
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = userInput;
    messagesArray.append(messageObj);
    json["messages"] = messagesArray;

    // 发送请求
    QByteArray postData = QJsonDocument(json).toJson();
    networkManager->post(request, postData);
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onReplyFinished);
}

// 网络响应回调
void MainWindow::onReplyFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray replyData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(replyData);
        if (!doc.isNull()) {
            QJsonObject json = doc.object();
            if (json.contains("choices")) {
                QJsonArray choices = json["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject choiceObj = choices[0].toObject();
                    if (choiceObj.contains("message")) {
                        QJsonObject messageObj = choiceObj["message"].toObject();
                        if (messageObj.contains("content")) {
                            QString response = messageObj["content"].toString();
                            ui->textEditChat->append(tr("Bot: %1").arg(response));
                        }
                    }
                }
            }
        }
    } else {
        ui->textEditChat->append(tr("Error: %1").arg(reply->errorString()));
    }
    reply->deleteLater();
}

// 绘制右下角透明蓝色三角形调整柄
void MainWindow::paintEvent(QPaintEvent *event) {
    QMainWindow::paintEvent(event); // 先让父类绘制

    int corner_padding = static_cast<int>(20 * m_scale); // 调整区域尺寸随DPI放大
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿

    // 定义右下角的蓝色三角形顶点坐标
    QPointF points[3];
    points[0] = QPointF(width(), height()); // 右下角顶点
    points[1] = QPointF(width() - corner_padding, height()); // 左边点（距离右边缘corner_padding）
    points[2] = QPointF(width(), height() - corner_padding); // 上边点（距离下边缘corner_padding）

    QColor handleColor("#4A90E2");
    handleColor.setAlpha(150); // 半透明
    painter.setBrush(handleColor);
    painter.setPen(Qt::NoPen); // 不显示边框

    painter.drawPolygon(points, 3);
}
