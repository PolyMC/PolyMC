// SPDX-License-Identifier: GPL-3.0-only

#include "ModrinthSourceDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

#include <utility>

#include "Application.h"
#include "BuildConfig.h"
#include "minecraft/AssetsUtils.h"

namespace
{
// Mods are code, so remote sources have to use HTTPS. HTTP stays allowed for
// local ones, like a Labrinth running on this machine.
QString sanitizeModrinthSourceUrl(const QString& text)
{
    auto normalized = Modrinth::normalizeApiUrl(text);
    if (normalized.isEmpty())
    {
        return {};
    }

    QUrl url(normalized);
    if (url.scheme() == "http" && !AssetsUtils::isLocalMetadataHost(url.host()))
    {
        url.setScheme("https");
    }
    return url.toString();
}

bool isUsableModrinthSourceUrl(const QString& text)
{
    QUrl url(text);
    return url.isValid() && !url.host().isEmpty() &&
           (url.scheme() == "https" || url.scheme() == "http");
}
}  // namespace

ModrinthSourceDialog::ModrinthSourceDialog(QWidget* parent,
                                           const Modrinth::Source& source,
                                           QStringList others)
    : QDialog(parent),
      m_name(new QLineEdit(source.name, this)),
      m_api_url(new QLineEdit(source.api_url, this)),
      m_website(new QLineEdit(source.website_url, this)),
      m_token(new QLineEdit(source.token, this)),
      m_test_result(new QLabel(this)),
      m_others(std::move(others))
{
    setWindowTitle(source.api_url.isEmpty() ? tr("Add Modrinth source")
                                            : tr("Edit Modrinth source"));
    setMinimumWidth(520);

    m_api_url->setPlaceholderText("https://example.org/_labrinth/v2");
    m_website->setPlaceholderText(tr("Defaults to the API URL's host"));
    m_token->setPlaceholderText(tr("None"));
    m_token->setEchoMode(QLineEdit::Password);

    auto* form = new QFormLayout;
    // The macOS style keeps fields at their size hint otherwise
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->addRow(tr("&Name:"), m_name);
    form->addRow(tr("&API URL:"), m_api_url);
    form->addRow(tr("&Website:"), m_website);
    form->addRow(tr("&Token:"), m_token);

    auto* token_note = new QLabel(
        tr("A token is only needed to access private projects, such as "
           "drafts. It is never sent anywhere except this API URL."),
        this);
    token_note->setWordWrap(true);

    m_test_result->setWordWrap(true);
    m_test_result->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    auto* test_button =
        buttons->addButton(tr("T&est"), QDialogButtonBox::ActionRole);
    connect(test_button, &QPushButton::clicked, this,
            &ModrinthSourceDialog::test);
    connect(buttons, &QDialogButtonBox::accepted, this,
            &ModrinthSourceDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this,
            &ModrinthSourceDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(token_note);
    layout->addWidget(m_test_result);
    layout->addStretch();
    layout->addWidget(buttons);
}

ModrinthSourceDialog::~ModrinthSourceDialog()
{
    stopTest();
}

Modrinth::Source ModrinthSourceDialog::source() const
{
    Modrinth::Source source;
    source.name = m_name->text().trimmed();
    source.api_url = sanitizeModrinthSourceUrl(m_api_url->text());
    source.website_url = sanitizeModrinthSourceUrl(m_website->text());
    source.token = m_token->text().trimmed();
    return source;
}

QString ModrinthSourceDialog::problem(bool saving) const
{
    auto entered = source();

    if (entered.api_url.isEmpty())
    {
        return tr("The API URL is missing.");
    }
    if (!isUsableModrinthSourceUrl(entered.api_url))
    {
        return tr("The API URL, \"%1\", isn't an http:// or https:// address.")
            .arg(m_api_url->text().trimmed());
    }
    if (!entered.website_url.isEmpty() &&
        !isUsableModrinthSourceUrl(entered.website_url))
    {
        return tr("The website, \"%1\", isn't an http:// or https:// address.")
            .arg(m_website->text().trimmed());
    }
    if (!saving)
    {
        return {};
    }
    if (entered.api_url ==
        Modrinth::normalizeApiUrl(BuildConfig.MODRINTH_PROD_URL))
    {
        return tr(
            "Official Modrinth is already included. Its token can be set in "
            "the API Keys tab.");
    }
    if (m_others.contains(entered.api_url))
    {
        return tr("Another source already has this API URL.");
    }
    return {};
}

void ModrinthSourceDialog::accept()
{
    auto reason = problem(true);
    if (!reason.isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid Modrinth source"), reason);
        return;
    }
    QDialog::accept();
}

QNetworkReply* ModrinthSourceDialog::get(const QUrl& url, const QString& token)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      APPLICATION->getUserAgent().toUtf8());
    // The token isn't saved yet, so it's added here instead of by the
    // downloads. Redirects aren't followed, they could take it elsewhere.
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::ManualRedirectPolicy);
    if (!token.isEmpty())
    {
        request.setRawHeader("Authorization", token.toUtf8());
    }
    return APPLICATION->network()->get(request);
}

void ModrinthSourceDialog::test()
{
    auto reason = problem(false);
    if (!reason.isEmpty())
    {
        m_test_result->setText(reason);
        return;
    }

    stopTest();
    auto entered = source();

    m_search_result.clear();
    m_test_result->setText(tr("Connecting to %1...").arg(entered.api_url));
    m_search_reply =
        get(QUrl(entered.api_url + "/search?limit=1"), entered.token);
    connect(m_search_reply, &QNetworkReply::finished, this,
            &ModrinthSourceDialog::onSearchFinished);
}

void ModrinthSourceDialog::onSearchFinished()
{
    auto* reply = m_search_reply.data();
    m_search_reply = nullptr;
    if (!reply)
    {
        return;
    }
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        m_test_result->setText(
            tr("Couldn't connect: %1").arg(reply->errorString()));
        return;
    }

    QJsonParseError parse_error{};
    auto obj = QJsonDocument::fromJson(reply->readAll(), &parse_error).object();
    if (parse_error.error != QJsonParseError::NoError ||
        !obj.value("hits").isArray())
    {
        m_test_result->setText(
            tr("This doesn't look like a Modrinth API. Make sure the URL "
               "points to the API root, which usually ends in /v2."));
        return;
    }

    m_search_result = tr("Connected successfully. %n project(s) available.",
                         nullptr, obj.value("total_hits").toInt());
    m_test_result->setText(m_search_result);

    auto entered = source();
    if (!entered.token.isEmpty())
    {
        m_user_reply = get(QUrl(entered.api_url + "/user"), entered.token);
        connect(m_user_reply, &QNetworkReply::finished, this,
                &ModrinthSourceDialog::onUserFinished);
    }
}

void ModrinthSourceDialog::onUserFinished()
{
    auto* reply = m_user_reply.data();
    m_user_reply = nullptr;
    if (!reply)
    {
        return;
    }
    reply->deleteLater();

    auto status =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status == 401 || status == 403)
    {
        m_test_result->setText(
            m_search_result + "\n" +
            tr("The token was rejected. It may have expired or belong to "
               "another site."));
        return;
    }
    if (reply->error() != QNetworkReply::NoError)
    {
        m_test_result->setText(
            m_search_result + "\n" +
            tr("Couldn't verify the token: %1").arg(reply->errorString()));
        return;
    }

    auto user = QJsonDocument::fromJson(reply->readAll()).object();
    m_test_result->setText(
        m_search_result + "\n" +
        tr("Signed in as %1.").arg(user.value("username").toString()));
}

void ModrinthSourceDialog::stopTest()
{
    for (auto* reply : {m_search_reply.data(), m_user_reply.data()})
    {
        if (reply)
        {
            reply->disconnect(this);
            reply->abort();
            reply->deleteLater();
        }
    }
    m_search_reply = nullptr;
    m_user_reply = nullptr;
}

#include "moc_ModrinthSourceDialog.cpp"
