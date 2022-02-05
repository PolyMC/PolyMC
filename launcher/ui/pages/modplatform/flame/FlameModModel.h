#pragma once

#include <RWStorage.h>

#include <QAbstractListModel>
#include <QIcon>
#include <QList>
#include <QMetaType>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QThreadPool>

#include <functional>
#include <net/NetJob.h>

#include "BaseInstance.h"
#include "FlameModPage.h"
#include "modplatform/flame/FlameModIndex.h"
#include <modplatform/flame/FlamePackIndex.h>

namespace FlameMod {


using LogoMap = QMap<QString, QIcon>;
using LogoCallback = std::function<void (QString)>;

class ListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ListModel(FlameModPage *parent);
    ~ListModel() override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool canFetchMore(const QModelIndex & parent) const override;
    void fetchMore(const QModelIndex & parent) override;

    void getLogo(const QString &logo, const QString &logoUrl, LogoCallback callback);
    void searchWithTerm(const QString &term, const int sort);

private slots:
    void performPaginatedSearch();

    void logoFailed(QString logo);
    void logoLoaded(QString logo, QIcon out);

    void searchRequestFinished();
    void searchRequestFailed(QString reason);

private:
    void requestLogo(QString file, QString url);

private:
    QList<IndexedPack> modpacks;
    QStringList m_failedLogos;
    QStringList m_loadingLogos;
    LogoMap m_logoMap;
    QMap<QString, LogoCallback> waitingCallbacks;

    QString currentSearchTerm;
    int currentSort = 0;
    int nextSearchOffset = 0;
    enum SearchState {
        None,
        CanPossiblyFetchMore,
        ResetRequested,
        Finished
    } searchState = None;
    NetJob::Ptr jobPtr;
    QByteArray response;
};

}