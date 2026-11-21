#pragma once

#include <QWidget>
#include <QString>
#include <QStringList>

namespace GuiUtil
{
QString fetchFlameKey(QWidget* parentWidget = nullptr);
QString uploadPaste(const QString& text, QWidget* parentWidget);
void setClipboardText(const QString& text);
QStringList BrowseForFiles(QString context, QString caption, QString filter,
                           QString defaultPath, QWidget* parentWidget);
QString BrowseForFile(QString context, QString caption, QString filter,
                      QString defaultPath, QWidget* parentWidget);
}  // namespace GuiUtil
