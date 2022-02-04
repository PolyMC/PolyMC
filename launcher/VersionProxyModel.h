#pragma once
#include <QAbstractProxyModel>
#include "BaseVersionList.h"

#include <Filter.h>

class VersionFilterModel;

class VersionProxyModel: public QAbstractProxyModel
{
    Q_OBJECT
public:

    enum Column
    {
        Name,
        ParentVersion,
        Branch,
        Type,
        Architecture,
        Path,
        Time
    };
    typedef QHash<BaseVersionList::ModelRoles, std::shared_ptr<Filter>> FilterMap;

public:
    VersionProxyModel ( QObject* parent = nullptr );
    ~VersionProxyModel() override = default;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    void setSourceModel(QAbstractItemModel *sourceModel) override;

    const FilterMap &filters() const;
    void setFilter(const BaseVersionList::ModelRoles column, Filter * filter);
    void clearFilters();
    QModelIndex getRecommended() const;
    QModelIndex getVersion(const QString & version) const;
    void setCurrentVersion(const QString &version);
private slots:

    void sourceDataChanged(const QModelIndex &source_top_left,const QModelIndex &source_bottom_right);

    void sourceAboutToBeReset();
    void sourceReset();

    void sourceRowsAboutToBeInserted(const QModelIndex &parent, int first, int last);
    void sourceRowsInserted(const QModelIndex &parent, int first, int last);

    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void sourceRowsRemoved(const QModelIndex &parent, int first, int last);

private:
    QList<Column> m_columns;
    FilterMap m_filters;
    BaseVersionList::RoleList roles;
    VersionFilterModel * filterModel;
    bool hasRecommended = false;
    bool hasLatest = false;
    QString m_currentVersion;
};
