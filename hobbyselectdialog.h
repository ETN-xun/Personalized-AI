#pragma once
#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QPropertyAnimation>
#include <QLabel> // 添加QLabel头文件
#include <QVBoxLayout> // 添加布局头文件

class HobbySelectDialog : public QDialog {
    Q_OBJECT
public:
    explicit HobbySelectDialog(QWidget *parent = nullptr);
    QStringList getSelectedHobbies() const;

signals:
    void hobbiesSelected(const QStringList &hobbies);

protected:
    void showEvent(QShowEvent *event) override;

private:
    void setupUI();
    void toggleHobby(QPushButton *button);

    QGridLayout *gridLayout;
    QPropertyAnimation *m_scaleAnimation;
    QList<QPushButton*> hobbyButtons;
    QStringList selectedHobbies;
};