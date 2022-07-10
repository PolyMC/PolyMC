#pragma once

#include <QtWidgets/QDialog>
#include <QtCore/QEventLoop>

#include "minecraft/auth/MinecraftAccount.h"
#include "tasks/Task.h"

namespace Ui
{
class DemoLoginDialog;
}

class DemoLoginDialog : public QDialog
{
    Q_OBJECT

   public:
    ~DemoLoginDialog();

    static MinecraftAccountPtr newAccount(QWidget *parent, QString message);

   private:
    explicit DemoLoginDialog(QWidget *parent = 0);

    void setUserInputsEnabled(bool enable);

   protected
       slots:
    void accept();

    void onTaskFailed(const QString &reason);
    void onTaskSucceeded();
    void onTaskStatus(const QString &status);
    void onTaskProgress(qint64 current, qint64 total);

    void on_userTextBox_textEdited(const QString &newText);

   private:
    Ui::DemoLoginDialog *ui;
    MinecraftAccountPtr m_account;
    Task::Ptr m_loginTask;
};