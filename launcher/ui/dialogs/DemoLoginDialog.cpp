#include "DemoLoginDialog.h"
#include "ui_DemoLoginDialog.h"

#include "minecraft/auth/AccountTask.h"

#include <QtWidgets/QPushButton>

DemoLoginDialog::DemoLoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::DemoLoginDialog)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

DemoLoginDialog::~DemoLoginDialog()
{
    delete ui;
}

// Stage 1: User interaction
void DemoLoginDialog::accept()
{
    setUserInputsEnabled(false);
    ui->progressBar->setVisible(true);

    // Setup the login task and start it
    m_account = MinecraftAccount::createDemo(ui->userTextBox->text());
    m_loginTask = m_account->loginDemo();
    connect(m_loginTask.get(), &Task::failed, this, &DemoLoginDialog::onTaskFailed);
    connect(m_loginTask.get(), &Task::succeeded, this, &DemoLoginDialog::onTaskSucceeded);
    connect(m_loginTask.get(), &Task::status, this, &DemoLoginDialog::onTaskStatus);
    connect(m_loginTask.get(), &Task::progress, this, &DemoLoginDialog::onTaskProgress);
    m_loginTask->start();
}

void DemoLoginDialog::setUserInputsEnabled(bool enable)
{
    ui->userTextBox->setEnabled(enable);
    ui->buttonBox->setEnabled(enable);
}

// Enable the OK button only when the textbox contains something.
void DemoLoginDialog::on_userTextBox_textEdited(const QString &newText)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)
        ->setEnabled(!newText.isEmpty());
}

void DemoLoginDialog::onTaskFailed(const QString &reason)
{
    // Set message
    auto lines = reason.split('\n');
    QString processed;
    for(auto line: lines) {
        if(line.size()) {
            processed += "<font color='red'>" + line + "</font><br />";
        }
        else {
            processed += "<br />";
        }
    }
    ui->label->setText(processed);

    // Re-enable user-interaction
    setUserInputsEnabled(true);
    ui->progressBar->setVisible(false);
}

void DemoLoginDialog::onTaskSucceeded()
{
    QDialog::accept();
}

void DemoLoginDialog::onTaskStatus(const QString &status)
{
    ui->label->setText(status);
}

void DemoLoginDialog::onTaskProgress(qint64 current, qint64 total)
{
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(current);
}

// Public interface
MinecraftAccountPtr DemoLoginDialog::newAccount(QWidget *parent, QString msg)
{
    DemoLoginDialog dlg(parent);
    dlg.ui->label->setText(msg);
    if (dlg.exec() == QDialog::Accepted)
    {
        return dlg.m_account;
    }
    return 0;
}