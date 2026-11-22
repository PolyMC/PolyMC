#pragma once

#include <modplatform/legacy_ftb/PackHelpers.h>

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QMap>
#include <QString>
#include <QVariant>

class LogoCache;

namespace LegacyFTB
{

class FilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    FilterModel(QObject* parent = Q_NULLPTR);
    enum Sorting
    {
        ByName,
        ByGameVersion
    };
    const QMap<QString, Sorting> getAvailableSortings();
    QString translateCurrentSorting();
    void setSorting(Sorting sorting);
    Sorting getCurrentSorting();

protected:
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left,
                  const QModelIndex& right) const override;

private:
    QMap<QString, Sorting> sortings;
    Sorting currentSorting;
};

class ListModel : public QAbstractListModel
{
    Q_OBJECT
private:
    ModpackList modpacks;
    LogoCache* m_logos;

    QString translatePackType(PackType type) const;
    void onLogoLoaded();

public:
    ListModel(QObject* parent);
    ~ListModel();
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void fill(ModpackList newModpacks);
    void addPack(const Modpack& modpack);
    void clear();
    void remove(int row);

    Modpack at(int row);
    QString logoPath(const QString& logo);
};

}  // namespace LegacyFTB
