#include "genderselectdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>

GenderSelectDialog::GenderSelectDialog(QWidget *parent)
    : QDialog(parent) {
    setupUI();
    setupStyle();
}

void GenderSelectDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *genderLayout = new QHBoxLayout();

    QPushButton *maleBtn = createGenderButton("男", "#4CAF50");
    QPushButton *femaleBtn = createGenderButton("女", "#E91E63");

    connect(maleBtn, &QPushButton::clicked, [this]() {
        selectedGender = "男";
        emit genderSelected(selectedGender);
        accept();
    });

    connect(femaleBtn, &QPushButton::clicked, [this]() {
        selectedGender = "女";
        emit genderSelected(selectedGender);
        accept();
    });

    genderLayout->addWidget(maleBtn);
    genderLayout->addWidget(femaleBtn);
    mainLayout->addLayout(genderLayout);
    setLayout(mainLayout);
    resize(400, 300);
}

QPushButton* GenderSelectDialog::createGenderButton(const QString &gender, const QString &color) {
    QPushButton *btn = new QPushButton(gender);
    btn->setFixedSize(120, 120);
    btn->setStyleSheet(QString(
                           "QPushButton {"
                           "  background-color: %1;"
                           "  border-radius: 60px;"
                           "  color: white;"
                           "  font-size: 24px;"
                           "}"
                           "QPushButton:hover {"
                           "  background-color: %2;"
                           "}"
                           ).arg(color, QColor(color).darker(120).name()));

    return btn;
}

void GenderSelectDialog::setupStyle() {
    setStyleSheet(
        "QDialog {"
        "  background-color: #f0f0f0;"
        "}"
        );
}

QString GenderSelectDialog::getSelectedGender() const {
    return selectedGender;
}
