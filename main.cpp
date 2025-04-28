#include "mainwindow.h"
#include "genderselectdialog.h"
#include <QApplication>
#include "hobbyselectdialog.h"
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDir> // 添加QDir头文件到顶部

int main(int argc ,char *argv[]) {
    QApplication a(argc, argv);
    
    // 设置应用程序信息（用于QStandardPaths）
    QCoreApplication::setOrganizationName("YourCompany");
    QCoreApplication::setApplicationName("Personalized-AI");
    
    // 获取AppData路径
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir appDataDir(appDataPath);
    
    // 如果目录不存在，创建它
    if (!appDataDir.exists()) {
        appDataDir.mkpath(".");
    }
    
    // 检查topics.json是否存在，如果不存在则从资源复制
    QFile topicsFile(appDataPath + "/topics.json");
    if (!topicsFile.exists()) {
        // 从应用程序目录复制文件
        QString exeDir = QCoreApplication::applicationDirPath();
        QFile::copy(exeDir + "/topics.json", appDataPath + "/topics.json");
    }
    
    QString gender;
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QFile file(path + "/gender_info.json");
    
    // 检查性别文件是否存在
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            gender = obj["gender"].toString();
        }
        file.close();
    }

    // 如果性别文件不存在或读取失败，显示选择对话框
    GenderSelectDialog genderDialog;
    if (gender.isEmpty()) {
        if (genderDialog.exec() != QDialog::Accepted) {
            return 0;
        }
        gender = genderDialog.getSelectedGender();
    }

    // 检查兴趣爱好文件是否存在
    QStringList hobbies;
    // 修改hobbies.json的读取方式
    QFile hobbyFile(path + "/hobbies.json");
    if (hobbyFile.exists() && hobbyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(hobbyFile.readAll());
        if (doc.isArray()) {
            QJsonArray array = doc.array();  // 需要QJsonArray定义
            for (const QJsonValue &value : array) {  // 修复遍历语法
                hobbies.append(value.toString());
            }
        }
        hobbyFile.close();
    }

    // 如果hobbies文件不存在或读取失败，显示选择对话框
    HobbySelectDialog hobbyDialog;
    if (hobbies.isEmpty()) {
        if (hobbyDialog.exec() != QDialog::Accepted) {
            return 0;
        }
        hobbies = hobbyDialog.getSelectedHobbies();
    }

    // 创建主窗口并传递信息
    MainWindow w;
    w.setUserGender(gender);
    w.setUserHobbies(hobbies); 
    w.show();

    return a.exec();
}
// 删除底部重复的头文件包含
