#pragma once

#include <QByteArray>
#include <QString>

namespace Markdown {

QString toHtml(const QByteArray& markdown);

QString lowerHtml(const QString& html);

}  // namespace Markdown
