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
// 添加以下两个头文件
#include <QJsonObject>
#include <QJsonDocument>

HobbySelectDialog::HobbySelectDialog(QWidget *parent)
    : QDialog(parent), m_scaleAnimation(new QPropertyAnimation(this, "geometry"))
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setupUI();
    adjustSize();
}

void HobbySelectDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // 调整主布局边距，让页面四周有更多空白
    mainLayout->setContentsMargins(70, 70, 70, 70);
    mainLayout->setSpacing(40);
    
    // 标题栏（参考GenderSelectDialog样式）
    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("选择兴趣爱好", this);
    // 优化标题样式，增加一些阴影效果
    titleLabel->setStyleSheet(R"(
        color: black; 
        font-size: 28px; 
        font-weight: bold;
        text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.1);
    )");
    titleLayout->addWidget(titleLabel);
    mainLayout->addLayout(titleLayout);

    // 兴趣按钮布局
    gridLayout = new QGridLayout();
    // 调整按钮之间的水平和垂直间距
    gridLayout->setHorizontalSpacing(20);
    gridLayout->setVerticalSpacing(20);
    QStringList hobbies = {
        "阅读", "运动", "音乐", "旅行", 
        "美食", "游戏", "编程", "摄影", 
        "绘画", "舞蹈", "电影", "写作", 
        "园艺", "手工", "收藏", "冥想"
    };
    
    // 恢复每行4个按钮的排版
    for(int i=0; i<hobbies.size(); i++) {
        QPushButton *btn = new QPushButton(hobbies[i], this);
        btn->setCheckable(true);
        // 优化按钮样式，调小按钮尺寸但保持字号不变
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #f0f0f0;
                border-radius: 15px;
                padding: 10px 15px;
                font-size: 20px;
                min-height: 50px;
                min-width: 100px;
                line-height: 30px;
                transition: all 0.3s ease;
                box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            }
            QPushButton:checked {
                background-color: #2196F3;
                color: white;
                box-shadow: 0 4px 8px rgba(33, 150, 243, 0.3);
            }
            QPushButton:hover {
                transform: translateY(-2px);
            }
        )");
        connect(btn, &QPushButton::clicked, [this, btn](){ toggleHobby(btn); });
        hobbyButtons.append(btn);
        gridLayout->addWidget(btn, i/4, i%4);
    }
    mainLayout->addLayout(gridLayout);

    // 确定按钮
    QPushButton *confirmBtn = new QPushButton("确定", this);
    confirmBtn->setFixedSize(240, 70);
    // 优化确定按钮样式，增加过渡效果
    confirmBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50;
            color: white;
            font-size: 24px;
            border-radius: 35px;
            box-shadow: 0 4px 8px rgba(76, 175, 80, 0.3);
            transition: all 0.3s ease;
        }
        QPushButton:hover {
            background-color: #45a049;
            transform: translateY(-2px);
        }
        QPushButton:pressed {
            transform: translateY(0);
            box-shadow: 0 2px 4px rgba(76, 175, 80, 0.3);
        }
    )");
    // 修改确定按钮的点击事件
    connect(confirmBtn, &QPushButton::clicked, this, [this]() {
        // 保存到JSON文件（已修复类型识别问题）
        QJsonArray hobbiesArray;
        foreach (const QString &hobby, selectedHobbies) {
            QJsonObject obj;
            obj["name"] = hobby;  // 保持QString类型赋值
            obj["weight"] = 1;  // 添加默认权重
            hobbiesArray.append(obj);
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

    // 在兴趣按钮布局后添加更多伸展空间，将确定按钮往下移
    mainLayout->addStretch(2); // 增加伸展系数，使按钮位于更下方
    
    // 确定按钮容器
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(confirmBtn);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout); // 将按钮布局添加到主布局底部
    mainLayout->addSpacing(30); // 在确定按钮下方添加额外空间
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
    
    // 增加窗口尺寸，从800x600改为900x700
    QSize size(900, 700);
    setFixedSize(size);
    
    m_scaleAnimation->setStartValue(QRect(screenRect.center().x(), screenRect.bottom(), 0, 0));
    m_scaleAnimation->setEndValue(QRect(screenRect.center().x()-450, screenRect.center().y()-350, 900, 700));
    m_scaleAnimation->setDuration(800);
    m_scaleAnimation->setEasingCurve(QEasingCurve::OutQuint);
    m_scaleAnimation->start();
    
    QDialog::showEvent(event);
}