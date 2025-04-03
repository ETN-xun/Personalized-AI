#pragma once
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QShowEvent>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>

class GenderSelectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GenderSelectDialog(QWidget *parent = nullptr);
    QString getSelectedGender() const;

signals:
    void genderSelected(const QString &gender);

protected:
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupUI();
    QString getButtonStyle(const QString &color, qreal scale) const;

    QString selectedGender;
    QPropertyAnimation *m_scaleAnimation;
    bool m_bDrag = false;
    QPoint dragPos;
    qreal m_scale = 1.0;
    QHBoxLayout *genderLayout; // 声明genderLayout为类成员
    int btnSize; // 声明btnSize为类成员
    QPushButton *maleBtn;
    QPushButton *femaleBtn;
};
