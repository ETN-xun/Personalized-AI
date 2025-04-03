#include "mainwindow.h"
#include "genderselectdialog.h"
#include <QApplication>

int main(int argc ,char *argv[]) {
    QApplication a(argc, argv);

    // 先显示性别选择对话框
    GenderSelectDialog genderDialog;
    if (genderDialog.exec() != QDialog::Accepted) {
        return 0; // 用户取消选择则退出
    }

    // 创建主窗口并传递性别信息
    MainWindow w;
    w.setUserGender(genderDialog.getSelectedGender());
    w.show();

    return a.exec();
}
