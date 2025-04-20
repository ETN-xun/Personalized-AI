#include "hobbyselectdialog.h"
#include <QScreen>
#include <QGuiApplication>
#include <QLabel> // 添加QLabel头文件
#include <QVBoxLayout> // 添加布局头文件

// 添加缺失的头文件引用
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

HobbySelectDialog::HobbySelectDialog(QWidget *parent)
    : QDialog(parent), m_scaleAnimation(new QPropertyAnimation(this, "geometry"))
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setupUI();
    adjustSize();
}

void HobbySelectDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    
    // 标题栏（参考GenderSelectDialog样式）
    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("选择兴趣爱好", this);
    titleLabel->setStyleSheet("color: black; font-size: 24px; font-weight: bold;");
    titleLayout->addWidget(titleLabel);
    mainLayout->addLayout(titleLayout);

    // 兴趣按钮布局
    gridLayout = new QGridLayout();
    QStringList hobbies = {"阅读", "运动", "音乐", "旅行", "美食", "游戏", "编程", "摄影"};
    
    for(int i=0; i<hobbies.size(); i++) {
        QPushButton *btn = new QPushButton(hobbies[i], this);
        btn->setCheckable(true);
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #f0f0f0;
                border-radius: 15px;
                padding: 20px;
                font-size: 18px;
                transition: all 0.3s ease;
            }
            QPushButton:checked {
                background-color: #2196F3;
                color: white;
            }
        )");
        connect(btn, &QPushButton::clicked, [this, btn](){ toggleHobby(btn); });
        hobbyButtons.append(btn);
        gridLayout->addWidget(btn, i/4, i%4);
    }
    mainLayout->addLayout(gridLayout);

    // 确定按钮
    QPushButton *confirmBtn = new QPushButton("确定", this);
    confirmBtn->setFixedSize(200, 60);
    confirmBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50;
            color: white;
            font-size: 20px;
            border-radius: 10px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
    )");
    // 修改确定按钮的点击事件
    connect(confirmBtn, &QPushButton::clicked, this, [this]() {
        // 保存到JSON文件（已修复类型识别问题）
        QJsonArray hobbiesArray;
        foreach (const QString &hobby, selectedHobbies) {
            hobbiesArray.append(hobby);
        }
        
        QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(path);
        QFile file(path + "/hobbies.json");
        if (file.open(QIODevice::WriteOnly)) {
            QJsonDocument doc(hobbiesArray);
            file.write(doc.toJson());
            file.close();
        }
        
        accept();
    });

    // 在兴趣按钮布局后添加伸展空间和确定按钮
    mainLayout->addStretch(); // 添加伸展空间使按钮位于底部
    
    // 确定按钮容器
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(confirmBtn);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout); // 将按钮布局添加到主布局底部
}

void HobbySelectDialog::toggleHobby(QPushButton *button) {
    QString hobby = button->text();
    if(selectedHobbies.contains(hobby)) {
        selectedHobbies.removeAll(hobby);
    } else {
        selectedHobbies.append(hobby);
    }
}

QStringList HobbySelectDialog::getSelectedHobbies() const {
    return selectedHobbies;
}

void HobbySelectDialog::showEvent(QShowEvent *event) {
    // 实现类似GenderSelectDialog的动画效果
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->availableGeometry();
    
    QSize size(800, 600);
    setFixedSize(size);
    
    m_scaleAnimation->setStartValue(QRect(screenRect.center().x(), screenRect.bottom(), 0, 0));
    m_scaleAnimation->setEndValue(QRect(screenRect.center().x()-400, screenRect.center().y()-300, 800, 600));
    m_scaleAnimation->setDuration(800);
    m_scaleAnimation->setEasingCurve(QEasingCurve::OutQuint);
    m_scaleAnimation->start();
    
    QDialog::showEvent(event);
}