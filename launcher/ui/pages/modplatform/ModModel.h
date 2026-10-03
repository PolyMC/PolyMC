#pragma once

#include <QAbstractListModel>

#include "modplatform/ModIndex.h"
#include "net/NetJob.h"

class ModPage;
class Version;

namespace ModPlatform {

using LogoMap = QMap<QString, QIcon>;
using LogoCallback = std::function<void (QString)>;

class ListModel : public QAbstractListModel {
    Q_OBJECT

   public:
    ListModel(ModPage* parent);
    ~ListModel() override;

    inline int rowCount(const QModelIndex& parent) const override { return modpacks.size(); };
    inline int columnCount(const QModelIndex& parent) const override { return 1; };
    inline Qt::ItemFlags flags(const QModelIndex& index) const override { return QAbstractListModel::flags(index); };

    QString debugName() const;

    /* Retrieve information from the model at a given index with the given role */
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    inline void setActiveJob(NetJob::Ptr ptr) { jobPtr = ptr; }
    inline NetJob* activeJob() { return jobPtr.get(); }

    /* Ask the API for more information */
    void fetchMore(const QModelIndex& parent) override;
    void refresh();
    void searchWithTerm(const QString& term, const int sort, const bool filter_changed);
    void requestModInfo(ModPlatform::IndexedPack& current, QModelIndex index);
    void requestModVersions(const ModPlatform::IndexedPack& current, QModelIndex index);

    virtual void loadIndexedPack(ModPlatform::IndexedPack& m, QJsonObject& obj) = 0;
    virtual void loadExtraPackInfo(ModPlatform::IndexedPack& m, QJsonObject& obj) {};
    virtual void loadIndexedPackVersions(ModPlatform::IndexedPack& m, QJsonArray& arr) = 0;

    void getLogo(const QString& logo, const QString& logoUrl, LogoCallback callback);

    inline bool canFetchMore(const QModelIndex& parent) const override { return searchState == CanPossiblyFetchMore; };

   public slots:
    void searchRequestFinished(QJsonDocument& doc);
    void searchRequestFailed(QString reason);

    void infoRequestFinished(QJsonDocument& doc, ModPlatform::IndexedPack& pack, const QModelIndex& index);

    void versionRequestSucceeded(QJsonDocument doc, QString addonId, const QModelIndex& index);

   protected slots:

    void logoFailed(QString logo);
    void logoLoaded(QString logo, QIcon out);

    void performPaginatedSearch();

   protected:
    virtual QJsonArray documentToArray(QJsonDocument& obj) const = 0;
    virtual const char** getSorts() const = 0;

    void requestLogo(QString file, QString url);

    inline std::list<Version> getMineVersions() const;

   protected:
    ModPage* m_parent;

    QList<ModPlatform::IndexedPack> modpacks;

    LogoMap m_logoMap;
    QMap<QString, LogoCallback> waitingCallbacks;
    QStringList m_failedLogos;
    QStringList m_loadingLogos;

    QString currentSearchTerm;
    int currentSort = 0;
    int nextSearchOffset = 0;
    enum SearchState { None, CanPossiblyFetchMore, ResetRequested, Finished } searchState = None;

    NetJob::Ptr jobPtr;
};
}  // namespace ModPlatform
