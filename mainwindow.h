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

private:
    enum Direction { UP, DOWN, LEFT, RIGHT, LEFT_TOP, LEFT_BOTTOM, RIGHT_TOP, RIGHT_BOTTOM, NONE };
    int getMouseRegion(const QPoint &pos) const;
    void updateCursorShape(const QPoint &pos);

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

    double m_rotationAngle = 0.0;

    // 新增函数声明
    void sendChatRequest(const QString &question, bool isOptimization);
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
