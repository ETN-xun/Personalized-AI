#pragma once
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QPoint>
#include <QScreen>
#include <QLabel>
#include <QDebug> // 新增调试输出支持

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    void setUserGender(const QString &gender); // 新增方法
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void on_pushButtonSend_clicked();
    void onReplyFinished(QNetworkReply *reply);

private:
    QString userGender; // 新增成员变量
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;
    QString apiKey;

    QPropertyAnimation *m_sizeAnimation; // 窗口动画
    bool m_bDrag = false; // 拖动状态
    QPoint dragPos; // 拖动坐标
    qreal m_scale = 1.0; // DPI缩放比例
    QWidget *titleBar; // 标题栏
    QLabel *titleLabel; // 标题文本
};
