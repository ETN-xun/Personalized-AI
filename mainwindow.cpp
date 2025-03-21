#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    networkManager = new QNetworkAccessManager(this);
    apiKey = "sk-eea6568b51c74da88e91f32f91485ab9"; // 替换为你的API密钥

    connect(ui->pushButtonSend, &QPushButton::clicked, this, &MainWindow::on_pushButtonSend_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonSend_clicked()
{
    QString userInput = ui->lineEditInput->text();
    if (userInput.isEmpty()) return;

    // 显示用户输入
    ui->textEditChat->append("You: " + userInput);
    ui->lineEditInput->clear();

    // 准备API请求
    QUrl url("https://api.deepseek.com/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    // 构造符合API要求的JSON请求体
    QJsonObject json;
    json["model"] = "deepseek-chat";  // 根据文档填写实际模型名称

    QJsonArray messagesArray;          // 消息数组
    QJsonObject messageObject;
    messageObject["role"] = "user";    // 角色为用户
    messageObject["content"] = userInput;  // 用户输入内容
    messagesArray.append(messageObject);    // 将消息对象添加到数组

    json["messages"] = messagesArray;  // 注意字段名是 messages（复数）

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    // 发送请求
    networkManager->post(request, data);
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onReplyFinished);
}
void MainWindow::onReplyFinished(QNetworkReply *reply)
{
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    QByteArray responseContent = reply->readAll();

    if (reply->error() == QNetworkReply::NoError) {
        // 打印完整的服务器响应
        qDebug() << "Server Response:" << responseContent;

        // 解析 JSON 响应
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseContent);
        if (jsonResponse.isNull()) {
            ui->textEditChat->append("Error: Invalid JSON response");
            return;
        }

        QJsonObject jsonObject = jsonResponse.object();

        // 检查响应格式
        if (jsonObject.contains("choices")) {
            QJsonArray choicesArray = jsonObject["choices"].toArray();
            if (!choicesArray.isEmpty()) {
                QJsonObject firstChoice = choicesArray[0].toObject();
                QString botReply = firstChoice["message"].toObject()["content"].toString();
                ui->textEditChat->append("Bot: " + botReply);
            } else {
                ui->textEditChat->append("Error: No choices in response");
            }
        } else if (jsonObject.contains("response")) {
            // 备用解析逻辑
            QString botReply = jsonObject["response"].toString();
            ui->textEditChat->append("Bot: " + botReply);
        } else {
            ui->textEditChat->append("Error: Unexpected response format");
        }
    } else {
        // 显示详细的错误信息
        QString errorMessage = QString("Error: HTTP %1\nResponse: %2")
                                   .arg(statusCode.toInt())
                                   .arg(QString(responseContent));
        ui->textEditChat->append(errorMessage);
    }

    reply->deleteLater();
}
