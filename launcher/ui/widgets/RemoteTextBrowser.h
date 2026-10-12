#pragma once

#include <QImage>
#include <QSet>
#include <QTextBrowser>
#include <QTextImageFormat>
#include <QUrl>
#include "net/NetJob.h"

class QTextDocument;

class RemoteTextBrowser : public QTextBrowser {
    Q_OBJECT
public:
    RemoteTextBrowser(QWidget *parent = nullptr);

protected:
    static constexpr int Marked = QTextFormat::UserProperty + 1;

    QVariant loadResource(int type, const QUrl &name) override;

public slots:
    void setHtml(const QString &text);

private:
    void rerender(QTextDocument *doc, const QUrl &url, const QSize &imageSize);
    QImage loadScaled(const QString &path);

    struct ImageFragment {
        int pos;
        QUrl url;
        QTextImageFormat fmt;
    };

    static QList<ImageFragment> collectFragments(const QTextDocument* doc);
    static void applyFormat(const QSize& size, const QVariant& mark, QTextDocument* doc, int pos,
                            QTextImageFormat fmt);

    QSet<QUrl> m_loading;
    QSet<QUrl> m_failed;

    QList<QPointer<NetJob>> m_jobs;
};
