#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QNetworkReply>
#include <QScreen>
#include <QGuiApplication>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMouseEvent>
#include <QPropertyAnimation>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_sizeAnimation(new QPropertyAnimation(this, "geometry")),
    titleBar(new QWidget(this)),
    titleLabel(new QLabel(titleBar)),
    networkManager(new QNetworkAccessManager(this))
{
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif

    ui->setupUi(this);

    // 无边框窗口设置
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(400, 300);

    // 标题栏初始化
    titleBar->setFixedHeight(40);
    titleBar->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #4A90E2,stop:1 #63B8FF);"
        "border-top-left-radius:8px;"
        "border-top-right-radius:8px;"
        );

    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(12, 0, 12, 0);

    titleLabel->setStyleSheet("color:white; font:bold 18px 'Microsoft Yahei';");
    titleLayout->addWidget(titleLabel);

    // 关闭按钮
    QPushButton *closeBtn = new QPushButton("×", titleBar);
    closeBtn->setFixedSize(30,30);
    closeBtn->setStyleSheet(R"(
        QPushButton{color:white; font:bold 20px; border-radius:15px;}
        QPushButton:hover{background:rgba(255,255,255,30);}
        QPushButton:pressed{background:rgba(255,255,255,60);}
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    titleLayout->addStretch();
    titleLayout->addWidget(closeBtn);

    // 主布局设置
    QWidget *central = ui->centralwidget;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(central->layout());
    mainLayout->insertWidget(0, titleBar);
    mainLayout->setContentsMargins(2,2,2,2);

    // 输入框样式
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

    // 发送按钮样式
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

    // 关联回车发送
    connect(ui->lineEditInput, &QLineEdit::returnPressed,
            ui->pushButtonSend, &QPushButton::click);
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::getMouseRegion(const QPoint &pos) const
{
    const int PADDING = 5;
    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();

    if(x < PADDING && y < PADDING)          return LEFT_TOP;
    if(x >= w-PADDING && y >= h-PADDING)    return RIGHT_BOTTOM;
    if(x < PADDING && y >= h-PADDING)       return LEFT_BOTTOM;
    if(x >= w-PADDING && y < PADDING)       return RIGHT_TOP;
    if(x < PADDING)                         return LEFT;
    if(x >= w-PADDING)                      return RIGHT;
    if(y < PADDING)                         return UP;
    if(y >= h-PADDING)                      return DOWN;
    return NONE;
}

void MainWindow::updateCursorShape(const QPoint &pos)
{
    switch(getMouseRegion(pos)) {
    case LEFT_TOP: case RIGHT_BOTTOM:
        setCursor(Qt::SizeFDiagCursor); break;
    case LEFT_BOTTOM: case RIGHT_TOP:
        setCursor(Qt::SizeBDiagCursor); break;
    case LEFT: case RIGHT:
        setCursor(Qt::SizeHorCursor); break;
    case UP: case DOWN:
        setCursor(Qt::SizeVerCursor); break;
    default:
        setCursor(Qt::ArrowCursor);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QPoint pos;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    pos = event->globalPos();
#else
    pos = event->globalPosition().toPoint();
#endif

    mousePressRegion = static_cast<Direction>(getMouseRegion(event->pos()));
    resizeStartPos = pos;
    resizeStartGeom = geometry();

    if(mousePressRegion == NONE && event->pos().y() < titleBar->height()) {
        dragPos = pos - frameGeometry().topLeft();
        m_bDrag = true;
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(mousePressRegion != NONE) {
        QPoint delta;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        delta = event->globalPos() - resizeStartPos;
#else
        delta = event->globalPosition().toPoint() - resizeStartPos;
#endif

        QRect newGeom = resizeStartGeom;

        switch(mousePressRegion) {
        case RIGHT_BOTTOM:
            newGeom.setWidth(resizeStartGeom.width() + delta.x());
            newGeom.setHeight(resizeStartGeom.height() + delta.y());
            break;
        case RIGHT:
            newGeom.setWidth(resizeStartGeom.width() + delta.x());
            break;
        case DOWN:
            newGeom.setHeight(resizeStartGeom.height() + delta.y());
            break;
        case LEFT_BOTTOM:
            newGeom.setLeft(resizeStartGeom.left() + delta.x());
            newGeom.setHeight(resizeStartGeom.height() + delta.y());
            break;
        case LEFT_TOP:
            newGeom.setTopLeft(resizeStartGeom.topLeft() + delta);
            break;
        case RIGHT_TOP:
            newGeom.setTopRight(resizeStartGeom.topRight() + delta);
            break;
        case LEFT:
            newGeom.setLeft(resizeStartGeom.left() + delta.x());
            break;
        case UP:
            newGeom.setTop(resizeStartGeom.top() + delta.y());
            break;
        }

        if(newGeom.width() < minimumWidth())
            newGeom.setWidth(minimumWidth());
        if(newGeom.height() < minimumHeight())
            newGeom.setHeight(minimumHeight());

        setGeometry(newGeom);
    } else {
        updateCursorShape(event->pos());
        if(m_bDrag) {
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            move(event->globalPos() - dragPos);
#else
            move(event->globalPosition().toPoint() - dragPos);
#endif
            event->accept();
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    mousePressRegion = NONE;
    m_bDrag = false;
    setCursor(Qt::ArrowCursor);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect available = screen->availableGeometry();
    QRect current = geometry();

    if(current.left() < available.left()) move(available.left(), y());
    if(current.top() < available.top()) move(x(), available.top());
    if(current.right() > available.right()) move(available.right()-width(), y());
    if(current.bottom() > available.bottom()) move(x(), available.bottom()-height());

    QMainWindow::resizeEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->availableGeometry();

    m_sizeAnimation->setDuration(800);
    m_sizeAnimation->setEasingCurve(QEasingCurve::OutQuint);

    QSize initSize(600, 450);
    QPoint centerPos = screenRect.center() - QPoint(initSize.width()/2, initSize.height()/2);

    m_sizeAnimation->setStartValue(QRect(centerPos.x(), screenRect.bottom(), 0, 0));
    m_sizeAnimation->setEndValue(QRect(centerPos, initSize));
    m_sizeAnimation->start();

    QMainWindow::showEvent(event);
}

void MainWindow::setUserGender(const QString &gender)
{
    userGender = gender;
    titleLabel->setText(tr("智能聊天 - 用户性别：%1").arg(gender));
}

void MainWindow::on_pushButtonSend_clicked()
{
    QString userInput = ui->lineEditInput->text().trimmed();
    if(userInput.isEmpty()) return;

    ui->textEditChat->append(tr("You: %1").arg(userInput));
    ui->lineEditInput->clear();

    QUrl url("https://api.deepseek.com/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer sk-eea6568b51c74da88e91f32f91485ab9");

    QJsonObject json;
    json["model"] = "deepseek-chat";
    json["temperature"] = 0.5;
    json["max_tokens"] = 2048;

    QJsonArray messagesArray;
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = userInput;
    messagesArray.append(messageObj);
    json["messages"] = messagesArray;

    networkManager->post(request, QJsonDocument(json).toJson());
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onReplyFinished);
}

void MainWindow::onReplyFinished(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if(!doc.isNull()) {
            QJsonObject json = doc.object();
            if(json.contains("choices")) {
                QString response = json["choices"].toArray()[0]
                                       .toObject()["message"].toObject()["content"].toString();
                ui->textEditChat->append(tr("Bot: %1").arg(response));
            }
        }
    } else {
        ui->textEditChat->append(tr("Error: %1").arg(reply->errorString()));
    }
    reply->deleteLater();
}
