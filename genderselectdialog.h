#ifndef GENDERSELECTDIALOG_H
#define GENDERSELECTDIALOG_H

#include <QDialog>
#include <QPushButton>

class GenderSelectDialog : public QDialog {
    Q_OBJECT
public:
    explicit GenderSelectDialog(QWidget *parent = nullptr);
    QString getSelectedGender() const;

signals:
    void genderSelected(const QString &gender);

private:
    void setupUI();
    void setupStyle();
    QPushButton *createGenderButton(const QString &gender, const QString &color);

    QString selectedGender;
};

#endif // GENDERSELECTDIALOG_H
