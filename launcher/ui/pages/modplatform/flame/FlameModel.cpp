#include "FlameModel.h"
#include <Json.h>
#include "Application.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QUrl>

#include <utility>

#include "net/Download.h"
#include "ui/pages/modplatform/LogoCache.h"

namespace Flame
{

ListModel::ListModel(QObject* parent)
    : QAbstractListModel(parent),
      m_logos(new LogoCache("FlamePacks", "Flame Icon Download",
                            "screenshot-placeholder", LogoCache::Skip::Nothing,
                            this))
{
    connect(m_logos, &LogoCache::logoLoaded, this, &ListModel::onLogoLoaded);
}

ListModel::~ListModel()
{
}

int ListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return modpacks.size();
}

int ListModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant ListModel::data(const QModelIndex& index, int role) const
{
    int pos = index.row();
    if (pos >= modpacks.size() || pos < 0 || !index.isValid())
    {
        return QString("INVALID INDEX %1").arg(pos);
    }

    IndexedPack pack = modpacks.at(pos);
    if (role == Qt::DisplayRole)
    {
        return pack.name;
    }
    else if (role == Qt::ToolTipRole)
    {
        if (pack.description.length() > 100)
        {
            // some magic to prevent to long tooltips and replace html
            // linebreaks
            QString edit = pack.description.left(97);
            edit = edit.left(edit.lastIndexOf("<br>"))
                       .left(edit.lastIndexOf(" "))
                       .append("...");
            return edit;
        }
        return pack.description;
    }
    else if (role == Qt::DecorationRole)
    {
        return m_logos->icon(pack.logoName, pack.logoUrl);
    }
    else if (role == Qt::UserRole)
    {
        QVariant v;
        v.setValue(pack);
        return v;
    }

    return QVariant();
}

bool ListModel::setData(const QModelIndex& index, const QVariant& value,
                        int /*role*/)
{
    int pos = index.row();
    if (pos >= modpacks.size() || pos < 0 || !index.isValid())
    {
        return false;
    }

    modpacks[pos] = value.value<Flame::IndexedPack>();

    return true;
}

void ListModel::onLogoLoaded(const QString& logo)
{
    for (int i = 0; i < modpacks.size(); i++)
    {
        if (modpacks[i].logoName == logo)
        {
            emit dataChanged(createIndex(i, 0), createIndex(i, 0),
                             {Qt::DecorationRole});
        }
    }
}

QString ListModel::logoPath(const QString& logo, const QString& url)
{
    return m_logos->path(logo, url);
}

Qt::ItemFlags ListModel::flags(const QModelIndex& index) const
{
    return QAbstractListModel::flags(index);
}

bool ListModel::canFetchMore(const QModelIndex& /*parent*/) const
{
    return searchState == CanPossiblyFetchMore;
}

void ListModel::fetchMore(const QModelIndex& parent)
{
    if (parent.isValid())
    {
        return;
    }
    if (nextSearchOffset == 0)
    {
        qWarning() << "fetchMore with 0 offset is wrong...";
        return;
    }
    performPaginatedSearch();
}

void ListModel::performPaginatedSearch()
{
    NetJob* netJob = new NetJob("Flame::Search", APPLICATION->network());
    auto searchUrl =
        QString(
            "https://api.curseforge.com/v1/mods/search?"
            "gameId=432&"
            "classId=4471&"
            "index=%1&"
            "pageSize=25&"
            "searchFilter=%2&"
            "sortField=%3&"
            "sortOrder=desc")
            // Encode the search, and fill everything in at once: with chained
            // arg() calls, something like %3 in the search would get replaced
            // by the next one
            .arg(QString::number(nextSearchOffset),
                 QString::fromUtf8(QUrl::toPercentEncoding(currentSearchTerm)),
                 QString::number(currentSort + 1));

    netJob->addNetAction(
        Net::Download::makeByteArray(QUrl(searchUrl), &response));
    jobPtr = netJob;
    jobPtr->start();
    QObject::connect(netJob, &NetJob::succeeded, this,
                     &ListModel::searchRequestFinished);
    QObject::connect(netJob, &NetJob::failed, this,
                     &ListModel::searchRequestFailed);
    QObject::connect(netJob, &NetJob::aborted, this,
                     &ListModel::onSearchAborted);
}

void ListModel::onSearchAborted()
{
    searchRequestFailed({});
}

void ListModel::searchWithTerm(const QString& term, int sort)
{
    if (currentSearchTerm == term &&
        currentSearchTerm.isNull() == term.isNull() && currentSort == sort)
    {
        return;
    }
    currentSearchTerm = term;
    currentSort = sort;
    // Only abort a search that is still going, a finished one never reports
    // back. Set the state first, aborting can report back right away.
    if (jobPtr && jobPtr->isRunning())
    {
        searchState = ResetRequested;
        jobPtr->abort();
        return;
    }
    else
    {
        beginResetModel();
        modpacks.clear();
        endResetModel();
        searchState = None;
    }
    nextSearchOffset = 0;
    performPaginatedSearch();
}

void Flame::ListModel::searchRequestFinished()
{
    jobPtr.reset();

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response from CurseForge at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << response;
        return;
    }

    QList<Flame::IndexedPack> newList;
    auto packs = Json::ensureArray(doc.object(), "data");
    for (auto packRaw : std::as_const(packs))
    {
        auto packObj = packRaw.toObject();

        Flame::IndexedPack pack;
        try
        {
            Flame::loadIndexedPack(pack, packObj);
            newList.append(pack);
        }
        catch (const JSONValidationError& e)
        {
            qWarning() << "Error while loading pack from CurseForge: "
                       << e.cause();
            continue;
        }
    }
    if (packs.size() < 25)
    {
        searchState = Finished;
    }
    else
    {
        nextSearchOffset += 25;
        searchState = CanPossiblyFetchMore;
    }

    // When you have a Qt build with assertions turned on, proceeding here will
    // abort the application
    if (newList.size() == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), modpacks.size(),
                    modpacks.size() + newList.size() - 1);
    modpacks.append(newList);
    endInsertRows();
}

void Flame::ListModel::searchRequestFailed(QString /*reason*/)
{
    jobPtr.reset();

    if (searchState == ResetRequested)
    {
        beginResetModel();
        modpacks.clear();
        endResetModel();

        nextSearchOffset = 0;
        performPaginatedSearch();
    }
    else
    {
        searchState = Finished;
    }
}

}  // namespace Flame

#include "moc_FlameModel.cpp"
