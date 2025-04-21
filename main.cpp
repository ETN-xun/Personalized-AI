#include "mainwindow.h"
#include "genderselectdialog.h"
#include <QApplication>
#include "hobbyselectdialog.h"
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
// 添加以下两个头文件
#include <QJsonArray>
#include <QJsonValue>

int main(int argc ,char *argv[]) {
    QApplication a(argc, argv);

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
#include <QStandardPaths>  // 添加QStandardPaths头文件
#include <QFile>           // 添加QFile头文件
#include <QJsonDocument>   // 添加JSON支持
#include <QJsonObject>     // 添加JSON对象支持
#include <QJsonArray>
#include <QJsonValue>
