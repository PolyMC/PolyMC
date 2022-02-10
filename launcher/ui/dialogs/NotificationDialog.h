#ifndef NOTIFICATIONDIALOG_H
#define NOTIFICATIONDIALOG_H

#include <QDialog>

#include "notifications/NotificationChecker.h"

namespace Ui {
class NotificationDialog;
}

class NotificationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NotificationDialog(const NotificationChecker::NotificationEntry &entry, QWidget *parent = nullptr);
    ~NotificationDialog() override;

    enum ExitCode
    {
        Normal,
        DontShowAgain
    };

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    Ui::NotificationDialog *ui;

    int m_dontShowAgainTime = 10;
    int m_closeTime = 5;

    QString m_dontShowAgainText;
    QString m_closeText;

private
slots:
    void on_dontShowAgainBtn_clicked();
    void on_closeBtn_clicked();
};

#endif // NOTIFICATIONDIALOG_H
