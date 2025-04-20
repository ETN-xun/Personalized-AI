#include "mainwindow.h"
#include "genderselectdialog.h"
#include <QApplication>
#include "hobbyselectdialog.h" // 添加头文件

int main(int argc ,char *argv[]) {
    QApplication a(argc, argv);

    // 显示性别选择对话框
    GenderSelectDialog genderDialog;
    if (genderDialog.exec() != QDialog::Accepted) {
        return 0;
    }

    // 显示兴趣爱好选择对话框
    HobbySelectDialog hobbyDialog;
    if (hobbyDialog.exec() != QDialog::Accepted) {
        return 0;
    }

    // 创建主窗口并传递信息
    MainWindow w;
    w.setUserGender(genderDialog.getSelectedGender());
    w.setUserHobbies(hobbyDialog.getSelectedHobbies()); // 新增：设置用户兴趣爱好
    w.show();

    return a.exec();
}
