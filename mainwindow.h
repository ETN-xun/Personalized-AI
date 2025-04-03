#pragma once
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QScreen>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QLabel>
#include <QNetworkReply>
#include <QPoint>
#include <QDebug> // 新增
#include <QPainter>
#include <QWindow> // 添加此头文件

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setUserGender(const QString &gender);

protected:
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override; // 新增

private slots:
    void on_pushButtonSend_clicked();
    void onReplyFinished(QNetworkReply *reply);

private:
    enum Direction { UP, DOWN, LEFT, RIGHT, LEFT_TOP, LEFT_BOTTOM, RIGHT_TOP, RIGHT_BOTTOM, NONE };
    int getMouseRegion(const QPoint &pos) const;
    void updateCursorShape(const QPoint &pos);

    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;
    QString apiKey = "sk-eea6568b51c74da88e91f32f91485ab9";
    QString userGender = "未选择";

    // 窗口控制
    Direction mousePressRegion = NONE;
    QPoint resizeStartPos;
    QRect resizeStartGeom;
    bool m_bDrag = false;
    QPoint dragPos;
    qreal m_scale = 1.0;
    QWidget *titleBar;
    QLabel *titleLabel;
    QPropertyAnimation *m_sizeAnimation;
    QWidget *m_resizeHandle; // 新增调整柄
};
