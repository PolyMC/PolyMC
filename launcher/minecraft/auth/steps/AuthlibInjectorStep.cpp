#include "AuthlibInjectorStep.h"
#include "Application.h"

#include <iostream>

#include <QNetworkRequest>
#include <QUuid>

AuthlibInjectorStep::AuthlibInjectorStep(AccountData* data) : AuthStep(data) {
}

AuthlibInjectorStep::~AuthlibInjectorStep() noexcept = default;

QString AuthlibInjectorStep::describe() {
    return tr("Fetching authlib injector API URL");
}


void AuthlibInjectorStep::perform() {
    // Default to the same as the base URL
    QUrl url;
    url.setScheme("https");
    url.setAuthority(m_data->authlibInjectorBaseUrl);
    qDebug() << url << url.toString() << url.isLocalFile();
    m_data->authlibInjectorApiLocation = url.toString();
    QNetworkRequest request = QNetworkRequest(url);
    m_reply.reset( APPLICATION->network()->get(request));
    connect(m_reply.get(), &QNetworkReply::finished, this, &AuthlibInjectorStep::onRequestDone);
    qDebug() << "Fetching authlib injector API URL";
}

void AuthlibInjectorStep::rehydrate() {
    // NOOP, for now. We only save bools and there's nothing to check.
}

void AuthlibInjectorStep::onRequestDone() {
    if (m_reply->hasRawHeader("x-authlib-injector-api-location"))
    {
        QString authlibInjectorApiLocationHeader = m_reply->rawHeader("x-authlib-injector-api-location");
        QUrl url = authlibInjectorApiLocationHeader;
        if (!url.isValid())
        {
            qDebug() << "Invalid Authlib Injector API URL specified by server: " << authlibInjectorApiLocationHeader;
            emit finished(AccountTaskState::STATE_FAILED_HARD, tr("Invalid authlib injector API URL"));
        }
        else
        {
            m_data->authlibInjectorApiLocation = authlibInjectorApiLocationHeader;
            qDebug() << "Authlib injector API URL: " << m_data->authlibInjectorApiLocation;
            emit finished(AccountTaskState::STATE_WORKING, tr("Fetched authlib injector API URL"));
        }
    }
    else
    {
        qDebug() << "Authlib injector API URL not found";
        emit finished(AccountTaskState::STATE_WORKING, tr("Authlib injector API URL not found, defaulting to the supplied base URL"));
    }
}
