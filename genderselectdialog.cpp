#include "genderselectdialog.h"
#include <QGuiApplication>
#include <QScreen>
#include <QHBoxLayout>
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardPaths> // 添加头文件
#include <QDir>

GenderSelectDialog::GenderSelectDialog(QWidget *parent)
    : QDialog(parent), m_scaleAnimation(new QPropertyAnimation(this, "geometry")),
    genderLayout(new QHBoxLayout) // 初始化genderLayout
{
    setAttribute(Qt::WA_StaticContents);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint); // 无边框窗口，支持自定义标题栏
    setupUI();
    adjustSize(); // 初始布局自适应
}

void GenderSelectDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40); // 恢复大边距（与原需求一致）
    mainLayout->setSpacing(30);

    // 自定义标题栏布局
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(20, 10, 20, 10); // 调整标题栏边距

    QLabel *titleLabel = new QLabel("选择性别", this);
    titleLabel->setFixedHeight(30);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet("color: black; font-size: 24px; font-weight: bold; padding-left: 10px;");

    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch(); // 关闭按钮右对齐

    // 关闭按钮样式优化（在白色背景上更协调）
    QPushButton *closeButton = new QPushButton("×", this);
    closeButton->setFixedSize(50, 50); // 缩小按钮尺寸
    closeButton->setStyleSheet(R"(
        QPushButton {
            color: #606060;
            font-size: 32px;
            background: transparent;
            padding: 5px;
            margin-right: 10px;
        }
        QPushButton:hover {
            color: red;
            background-color: rgba(255, 0, 0, 0.1);
        }
        QPushButton:pressed {
            color: darkred;
        }
    )");
    titleLayout->addWidget(closeButton);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    mainLayout->addLayout(titleLayout);

    // 性别按钮布局
    genderLayout->setSpacing(50); // 按钮间距
    genderLayout->setAlignment(Qt::AlignCenter); // 按钮居中
    mainLayout->addLayout(genderLayout);

    // 初始化按钮（样式在showEvent中动态设置）
    maleBtn = new QPushButton(this);
    femaleBtn = new QPushButton(this);

    // 设置窗口背景为白色
    this->setStyleSheet("background-color: white;");
}

QString GenderSelectDialog::getButtonStyle(const QString &color, qreal scale) const
{
    QColor baseColor(color);
    QColor lighterColor = baseColor.lighter(120);
    QColor darkerColor = baseColor.darker(120);

    int btnSize = 140 * scale; // 按钮尺寸适配DPI
    qreal radius = btnSize / 2.0;
    int fontSize = 28 * scale;

    return QString(R"(
        QPushButton {
            background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 %1, stop:1 %2);
            border-radius: %3px;
            color: black; /* 按钮文字颜色改为黑色 */
            font-size: %4px;
            font-weight: bold;
            border: 2px solid %5;
            padding: 10px;
            transition: all 0.3s ease;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1); /* 调整阴影透明度 */
        }
        QPushButton:hover {
            background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 %2, stop:1 %1);
            transform: scale(1.05);
        }
        QPushButton:pressed {
            background-color: %6;
            transform: scale(0.95);
        }
    )").arg(baseColor.name(), lighterColor.name(),
             QString::number(radius),
             QString::number(fontSize),
             lighterColor.name(), // 边框颜色
             darkerColor.name()); // 按下时背景色
}

void GenderSelectDialog::showEvent(QShowEvent *event)
{
    QScreen *currentScreen = QGuiApplication::primaryScreen();
    if (currentScreen) {
// 计算缩放比例
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        m_scale = currentScreen->devicePixelRatio();
#else
        qreal dpiX = currentScreen->logicalDotsPerInchX();
        m_scale = dpiX / 96.0;
#endif

        const int desiredWidth = 400;
        const int desiredHeight = 300;
        const int scaledWidth = desiredWidth * m_scale;
        const int scaledHeight = desiredHeight * m_scale;

        setFixedSize(scaledWidth, scaledHeight); // 固定尺寸，适配DPI

        // 动画起始和结束位置
        QRect screenRect = currentScreen->availableGeometry();
        int targetX = screenRect.x() + (screenRect.width() - scaledWidth) / 2;
        int targetY = screenRect.y() + (screenRect.height() - scaledHeight) / 2;
        int startY = screenRect.bottom();

        m_scaleAnimation->setStartValue(QRect(targetX, startY, 0, 0));
        m_scaleAnimation->setEndValue(QRect(targetX, targetY, scaledWidth, scaledHeight));
        m_scaleAnimation->setDuration(800);
        m_scaleAnimation->setEasingCurve(QEasingCurve::OutQuint);

        // 计算按钮尺寸
        btnSize = 140 * m_scale; // 将btnSize定义为类成员，并在此处赋值

        // 设置按钮文本和样式
        maleBtn->setText("♂ 男");
        maleBtn->setFixedSize(btnSize, btnSize); // 使用类成员btnSize
        maleBtn->setStyleSheet(getButtonStyle("#2196F3", m_scale));

        femaleBtn->setText("♀ 女");
        femaleBtn->setFixedSize(btnSize, btnSize); // 使用类成员btnSize
        femaleBtn->setStyleSheet(getButtonStyle("#E91E63", m_scale));

        // 添加按钮到布局（确保布局已正确声明为成员变量）
        genderLayout->addWidget(maleBtn);
        genderLayout->addWidget(femaleBtn);
    }

    // 连接按钮信号（确保样式已设置）
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); // 声明并赋值 path
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    connect(maleBtn, &QPushButton::clicked, this, [this, path]() { // 捕获 path
        selectedGender = "男";
        emit genderSelected(selectedGender);
        // 保存选择结果到JSON文件
        QFile file(path + "/gender_info.json"); 
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonObject jsonObj;
            jsonObj["gender"] = selectedGender;
            QJsonDocument doc(jsonObj);
            file.write(doc.toJson());
            file.close();
        }
        accept();
    });

    connect(femaleBtn, &QPushButton::clicked, this, [this, path]() { // 捕获 path
        selectedGender = "女";
        emit genderSelected(selectedGender);
        // 保存选择结果到JSON文件
        QFile file(path + "/gender_info.json"); 
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonObject jsonObj;
            jsonObj["gender"] = selectedGender;
            QJsonDocument doc(jsonObj);
            file.write(doc.toJson());
            file.close();
        }
        accept();
    });

    m_scaleAnimation->start();
    QDialog::showEvent(event);
}

void GenderSelectDialog::mousePressEvent(QMouseEvent *event)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    const QPointF pos = event->position();
#else
    const QPoint pos = event->pos();
#endif

    const int yPos = pos.y();
    const int titleHeight = 30; // 标题栏高度

    if (yPos < titleHeight) {
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
#else
        dragPos = event->globalPos() - frameGeometry().topLeft();
#endif
        m_bDrag = true;
        event->accept();
    } else {
        QDialog::mousePressEvent(event);
    }
}

void GenderSelectDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bDrag) {
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        move(event->globalPosition().toPoint() - dragPos);
#else
        move(event->globalPos() - dragPos);
#endif
        event->accept();
    }
}

void GenderSelectDialog::mouseReleaseEvent(QMouseEvent *event)
{
    m_bDrag = false;
    event->accept();
}

QString GenderSelectDialog::getSelectedGender() const
{
    return selectedGender;
}
