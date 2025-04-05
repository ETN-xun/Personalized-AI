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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    titleBar(new QWidget(this)),
    titleLabel(new QLabel(titleBar)),
    networkManager(new QNetworkAccessManager(this)),
    closeBtn(nullptr),
    rotationAnimation(new QPropertyAnimation(this, "rotationAngle")),
    m_sizeAnimation(new QPropertyAnimation(this, "geometry")),
    loadIndicator(new QLabel(titleBar)),
    m_rotationAngle(0.0),
    isLoading(false)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

    titleBar->setFixedHeight(40 * m_scale);
    titleBar->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #4A90E2,stop:1 #63B8FF);"
        "border-top-left-radius:8px;"
        "border-top-right-radius:8px;"
        );

    // 标题栏布局
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar); // 局部变量
    titleLayout->setContentsMargins(12 * m_scale, 0, 12 * m_scale, 0);
    titleLayout->setSpacing(10 * m_scale);

    titleLabel->setStyleSheet("color:white; font:bold 14px 'Microsoft YaHei';");
    titleLayout->addWidget(titleLabel);

    // 加载指示器
    loadIndicator->setFixedSize(18 * m_scale, 18 * m_scale);
    loadIndicator->setScaledContents(true);
    loadIndicator->setStyleSheet("background: transparent;");
    loadIndicator->clear();

    // 关闭按钮
    closeBtn = new QPushButton("×", titleBar);
    closeBtn->setFixedSize(22 * m_scale, 22 * m_scale);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   color: white;"
        "   font-size: 16px;"
        "   border-radius: 11px;"
        "   background: transparent;"
        "}"
        "QPushButton:hover { background: rgba(255,255,255,30); }"
        "QPushButton:pressed { background: rgba(255,255,255,60); }"
        );
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    // 布局调整
    titleLayout->addStretch();
    titleLayout->addSpacing(15 * m_scale);
    titleLayout->addWidget(loadIndicator);
    titleLayout->addWidget(closeBtn);
    titleLayout->setAlignment(loadIndicator, Qt::AlignVCenter);
    titleLayout->setAlignment(closeBtn, Qt::AlignVCenter);

    // 连接动画更新
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

        // 绘制蓝色外环
        painter.setPen(QPen(QColor("#4A90E2"), lineWidth));
        painter.drawEllipse(0, 0, indicatorSize, indicatorSize);

        // 黄色扇形旋转
        painter.setPen(QPen(QColor("#FFD700"), lineWidth));
        painter.drawArc(
            0, 0, indicatorSize, indicatorSize,
            static_cast<int>(rotationAngle() * 16),
            -90 * 16
            );

        loadIndicator->setPixmap(pixmap);
    });

    // 主布局设置
    QWidget *central = ui->centralwidget;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(central->layout());
    mainLayout->insertWidget(0, titleBar);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 窗口弹出动画
    m_sizeAnimation->setDuration(800);
    m_sizeAnimation->setEasingCurve(QEasingCurve::OutQuint);

    // 旋转动画配置
    rotationAnimation->setDuration(1000);
    rotationAnimation->setStartValue(0);
    rotationAnimation->setEndValue(360);
    rotationAnimation->setLoopCount(-1);
    rotationAnimation->setEasingCurve(QEasingCurve::Linear);

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
}

MainWindow::~MainWindow() {
    delete ui;
    delete loadIndicator;
    delete closeBtn;
    delete rotationAnimation;
    delete m_sizeAnimation;
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->availableGeometry();
    QSize initSize(600, 400);

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    m_scale = screen->devicePixelRatio();
#else
    qreal dpiX = screen->logicalDotsPerInchX();
    m_scale = dpiX / 96.0;
#endif

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

void MainWindow::on_pushButtonSend_clicked() {
    QString userInput = ui->lineEditInput->text().trimmed();
    if (userInput.isEmpty()) return;

    ui->textEditChat->append(tr("You: %1").arg(userInput));
    ui->lineEditInput->clear();

    // 启动画
    isLoading = true;
    setRotationAngle(0.0);
    rotationAnimation->start();

    QUrl url("https://api.deepseek.com/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    QJsonObject json{
        {"model", "deepseek-chat"},
        {"temperature", 0.5},
        {"max_tokens", 2048},
        {"messages", QJsonArray{
                         QJsonObject{
                             {"role", "user"},
                             {"content", userInput}
                         }
                     }}
    };

    networkManager->post(request, QJsonDocument(json).toJson());
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onReplyFinished);
}

void MainWindow::onReplyFinished(QNetworkReply *reply) {
    isLoading = false;
    rotationAnimation->stop();
    setRotationAngle(0.0);
    loadIndicator->clear();

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isObject() && doc.object().contains("choices")) {
            QJsonObject messageObj = doc.object()["choices"].toArray().at(0).toObject()["message"].toObject();
            if (messageObj.contains("content")) {
                ui->textEditChat->append(tr("Bot: %1").arg(messageObj["content"].toString()));
            }
        }
    } else {
        ui->textEditChat->append(tr("Error: %1").arg(reply->errorString()));
    }
    reply->deleteLater();
    update();
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

// 其他函数保持不变，但修正resizeEvent中的问题：

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    QScreen *screen = windowHandle()->screen();
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    m_scale = screen->devicePixelRatio();
#else
    qreal dpiX = screen->logicalDotsPerInchX();
    m_scale = dpiX / 96.0;
#endif

    // 动态调整标题栏部件
    titleBar->setFixedHeight(40 * m_scale);
    loadIndicator->setFixedSize(18 * m_scale, 18 * m_scale);
    closeBtn->setFixedSize(22 * m_scale, 22 * m_scale);

    // 获取标题栏布局并更新参数
    auto *titleLayout = qobject_cast<QHBoxLayout*>(titleBar->layout());
    if (titleLayout) {
        titleLayout->setContentsMargins(12 * m_scale, 0, 12 * m_scale, 0);
        titleLayout->setSpacing(10 * m_scale);
    }
}

int MainWindow::getMouseRegion(const QPoint &pos) const {
    const int PADDING = static_cast<int>(20 * m_scale);
    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();

    if (y < titleBar->height()) return NONE;

    if (x < PADDING && y < PADDING) return LEFT_TOP;
    if (x >= w - PADDING && y >= h - PADDING) return RIGHT_BOTTOM;
    if (x < PADDING && y >= h - PADDING) return LEFT_BOTTOM;
    if (x >= w - PADDING && y < PADDING) return RIGHT_TOP;

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

    if (event->y() < titleBar->height()) {
        dragPos = pos - frameGeometry().topLeft();
        m_bDrag = true;
        event->accept();
        return;
    }

    mousePressRegion = static_cast<Direction>(getMouseRegion(event->pos()));
    resizeStartPos = pos;
    resizeStartGeom = geometry();

    if (mousePressRegion == NONE && event->y() < titleBar->height()) {
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

void MainWindow::setUserGender(const QString &gender) {
    userGender = gender;
    titleLabel->setText(tr("智能聊天 - 用户性别：%1").arg(gender));
}
