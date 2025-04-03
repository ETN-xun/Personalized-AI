// mainwindow.h
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>  // 包含 JSON 序列化支持
#include <QJsonObject>    // 必须添加
#include <QJsonArray>     // 必须添加

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    void setUserGender(const QString &gender); // 新增方法
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonSend_clicked();
    void onReplyFinished(QNetworkReply *reply);

private:
    QString userGender; // 新增成员变量
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;
    QString apiKey;
};
