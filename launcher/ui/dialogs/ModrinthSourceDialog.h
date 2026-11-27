// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QDialog>
#include <QPointer>
#include <QString>
#include <QStringList>

#include "modplatform/modrinth/ModrinthSource.h"

class QLabel;
class QLineEdit;
class QNetworkReply;
class QUrl;

// Adds or edits a Modrinth-compatible source, and tests it
class ModrinthSourceDialog : public QDialog
{
    Q_OBJECT

public:
    // others are the API URLs of the other sources, which can't be reused
    ModrinthSourceDialog(QWidget* parent, const Modrinth::Source& source,
                         QStringList others);
    ~ModrinthSourceDialog() override;

    // The source as entered, with normalized URLs
    Modrinth::Source source() const;

public slots:
    void accept() override;

private:
    // Why the source can't be saved, or tested if not saving, empty if it
    // can
    QString problem(bool saving) const;

    void test();
    void onSearchFinished();
    void onUserFinished();
    void stopTest();
    QNetworkReply* get(const QUrl& url, const QString& token);

    QLineEdit* m_name;
    QLineEdit* m_api_url;
    QLineEdit* m_website;
    QLineEdit* m_token;
    QLabel* m_test_result;

    QStringList m_others;

    QPointer<QNetworkReply> m_search_reply;
    QPointer<QNetworkReply> m_user_reply;
    QString m_search_result;
};
