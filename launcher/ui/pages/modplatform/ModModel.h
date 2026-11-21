#pragma once

#include <QAbstractListModel>

#include "modplatform/ModIndex.h"
#include "net/NetJob.h"

class LogoCache;
class ModPage;
class Version;

namespace ModPlatform
{

class ListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ListModel(ModPage* parent);
    ~ListModel() override;

    inline int rowCount(const QModelIndex& /*parent*/) const override
    {
        return modpacks.size();
    };
    inline int columnCount(const QModelIndex& /*parent*/) const override
    {
        return 1;
    };
    inline Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        return QAbstractListModel::flags(index);
    };

    QString debugName() const;

    /* Retrieve information from the model at a given index with the given role
     */
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role) override;

    inline NetJob* activeJob()
    {
        return jobPtr.get();
    }

    /* Ask the API for more information */
    void fetchMore(const QModelIndex& parent) override;
    void refresh();
    void searchWithTerm(const QString& term, const int sort,
                        const bool filter_changed);
    void requestModInfo(const ModPlatform::IndexedPack& current,
                        QModelIndex index);
    void requestModVersions(const ModPlatform::IndexedPack& current,
                            QModelIndex index);

    virtual void loadIndexedPack(ModPlatform::IndexedPack& m,
                                 QJsonObject& obj) = 0;
    virtual void loadExtraPackInfo(ModPlatform::IndexedPack& /*m*/,
                                   QJsonObject& /*obj*/) {};
    virtual void loadIndexedPackVersions(ModPlatform::IndexedPack& m,
                                         QJsonArray& arr) = 0;

    inline bool canFetchMore(const QModelIndex& /*parent*/) const override
    {
        return searchState == CanPossiblyFetchMore;
    };

public slots:
    void searchRequestSucceeded();
    void searchRequestFailed(QString reason);
    void searchRequestAborted();

    void infoRequestSucceeded();

    void versionRequestSucceeded();

protected slots:
    void performPaginatedSearch();

protected:
    virtual QJsonArray documentToArray(QJsonDocument& obj) const = 0;
    virtual const char** getSorts() const = 0;

    inline std::list<Version> getMineVersions() const;

private:
    void onLogoLoaded(const QString& logo);
    void searchRequestFinished(QJsonDocument& doc);

protected:
    ModPage* m_parent;

    QList<ModPlatform::IndexedPack> modpacks;

    LogoCache* m_logos;

    QString currentSearchTerm;
    int currentSort = 0;
    int nextSearchOffset = 0;
    enum SearchState
    {
        None,
        CanPossiblyFetchMore,
        ResetRequested,
        Finished
    } searchState = None;

    NetJob::Ptr jobPtr;
    QByteArray m_search_response;

    NetJob::Ptr m_info_job;
    QByteArray m_info_response;
    QVariant m_info_addon;
    QPersistentModelIndex m_info_index;

    NetJob::Ptr m_versions_job;
    QByteArray m_versions_response;
    QString m_versions_addon;
    QPersistentModelIndex m_versions_index;
};
}  // namespace ModPlatform
