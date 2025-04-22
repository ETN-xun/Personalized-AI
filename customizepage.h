#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class CustomizePage : public QWidget
{
    Q_OBJECT

public:
    explicit CustomizePage(QWidget *parent = nullptr);
    ~CustomizePage();

private:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *closeBtn;

private slots:
    void setupUI();
};