#pragma once

#include <QWidget>

namespace GuiUtil
{
QString uploadPaste(const QString &text, QWidget *parentWidget);
void setClipboardText(const QString &text);
QStringList BrowseForFiles(const QString& context, const QString& caption, const QString& filter, const QString& defaultPath, QWidget *parentWidget);
QString BrowseForFile(const QString& context, const QString& caption, const QString& filter, const QString& defaultPath, QWidget *parentWidget);
}
