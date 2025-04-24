#pragma once
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QScreen>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QLabel>
#include <QNetworkReply>
#include <QPoint>
#include <QDebug>
#include <QWidget>
#include <QPushButton>
#include <QWindow>
#include <QtGui/qwindowdefs.h>
#include <QPainter>
#include <QListWidget> // 添加列表控件头文件
#include <QJsonArray> // 添加JSON数组头文件
#include <QJsonObject> // 添加JSON对象头文件

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY(double rotationAngle READ rotationAngle WRITE setRotationAngle)

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setUserGender(const QString &gender);
    void setUserHobbies(const QStringList &hobbies); // 新增：设置用户兴趣爱好

    // 属性读写函数
    double rotationAngle() const { return m_rotationAngle; }
    void setRotationAngle(double angle) {
        m_rotationAngle = angle;
        update();
        if (loadIndicator) {
            loadIndicator->update();
        }
    }

protected:
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void on_pushButtonSend_clicked();
    void onReplyFinished(QNetworkReply *reply);
    void toggleMaximize();
    void onQuestionButtonClicked(); // 新增：问题按钮点击处理
    void openCustomizePage();
    void createNewChat(); // 新增：创建新会话
    void loadChatHistory(int index); // 新增：加载历史会话

private:
    QPushButton *customizeBtn;
    enum Direction { UP, DOWN, LEFT, RIGHT, LEFT_TOP, LEFT_BOTTOM, RIGHT_TOP, RIGHT_BOTTOM, NONE };
    int getMouseRegion(const QPoint &pos) const;
    void updateCursorShape(const QPoint &pos);
    void createQuestionButtons(); // 新增：创建问题按钮
    void saveChatHistory(); // 新增：保存聊天历史
    void loadChatHistories(); // 新增：加载所有聊天历史

    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;
    QString apiKey = "sk-eea6568b51c74da88e91f32f91485ab9"; // 确保使用正确的API密钥
    QString userGender = "未选择"; // 确保声明

    // 窗口控制
    Direction mousePressRegion = NONE;
    QPoint resizeStartPos;
    QRect resizeStartGeom;
    bool m_bDrag = false;
    QPoint dragPos;
    qreal m_scale = 1.0; // 初始化为默认值
    QWidget *titleBar;
    QLabel *titleLabel;
    QPushButton *closeBtn;
    QPushButton *minBtn;
    QPushButton *portraitBtn; // 新增画像按钮声明
    QWidget *portraitWindow;  // 新增空白窗口声明
    QPushButton *maxBtn;
    QPropertyAnimation *rotationAnimation;
    QPropertyAnimation *m_sizeAnimation;
    QLabel *loadIndicator;
    QStringList userHobbies; // 新增：存储用户选择的兴趣爱好
    bool isLoading = false;
    
    // 新增：问题按钮相关
    QList<QPushButton*> questionButtons;
    QStringList questions;
    bool buttonsShown = false;
    bool hasAskedQuestion = false;  // 新增：记录用户是否已经提过问题

    double m_rotationAngle = 0.0;

    // 新增函数声明
    void sendChatRequest(const QString &question, bool isOptimization);
    
    // 新增：侧边栏和会话历史相关
    QWidget *sideBar;
    QListWidget *chatHistoryList;
    QPushButton *newChatBtn;
    QString currentChatId;
    QList<QJsonObject> chatHistories;
};


class PieChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit PieChartWidget(QWidget *parent = nullptr) : QWidget(parent) {}
    void setHobbiesWithWeights(const QList<QPair<QString, int>> &hobbies);
    void setHobbies(const QStringList &hobbies) { m_hobbies = hobbies; update(); }
private:
    QList<QPair<QString, int>> m_hobbiesWithWeights;
    QStringList m_hobbies;  // 添加成员变量
protected:
    void paintEvent(QPaintEvent *) override;
};
