#include "ModModel.h"

#include "BuildConfig.h"
#include "Json.h"
#include "LogoCache.h"
#include "ModPage.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "ui/dialogs/ModDownloadDialog.h"

#include "ui/widgets/ProjectItem.h"

#include <QMessageBox>

namespace ModPlatform
{

namespace
{
void stopJob(NetJob* job, QObject* receiver)
{
    if (job && job->isRunning())
    {
        job->disconnect(receiver);
        job->abort();
    }
}
}  // namespace

ListModel::ListModel(ModPage* parent)
    : QAbstractListModel(parent),
      m_parent(parent),
      m_logos(new LogoCache(
          parent->metaEntryBase(), parent->debugName() + " Icon Download",
          "screenshot-placeholder", LogoCache::Skip::EmptyUrl, this))
{
    connect(m_logos, &LogoCache::logoLoaded, this, &ListModel::onLogoLoaded);
}

ListModel::~ListModel()
{
    stopJob(jobPtr.get(), this);
    stopJob(m_info_job.get(), this);
    stopJob(m_versions_job.get(), this);
}

QString ListModel::debugName() const
{
    return m_parent->debugName();
}

/******** Make data requests ********/

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

QVariant ListModel::data(const QModelIndex& index, int role) const
{
    int pos = index.row();
    if (pos >= modpacks.size() || pos < 0 || !index.isValid())
    {
        return QString("INVALID INDEX %1").arg(pos);
    }

    ModPlatform::IndexedPack pack = modpacks.at(pos);
    switch (role)
    {
        case Qt::ToolTipRole:
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
        case Qt::DecorationRole:
        {
            return m_logos->icon(pack.logoName, pack.logoUrl);
        }
        case Qt::SizeHintRole:
            return QSize(0, 58);
        case Qt::UserRole:
        {
            QVariant v;
            v.setValue(pack);
            return v;
        }
            // Custom data
        case UserDataTypes::TITLE:
            return pack.name;
        case UserDataTypes::DESCRIPTION:
            return pack.description;
        case UserDataTypes::SELECTED:
            return m_parent->getDialog()->isModSelected(pack.name);
        default:
            break;
    }

    return {};
}

bool ListModel::setData(const QModelIndex& index, const QVariant& value,
                        int /*role*/)
{
    int pos = index.row();
    if (pos >= modpacks.size() || pos < 0 || !index.isValid())
    {
        return false;
    }

    modpacks[pos] = value.value<ModPlatform::IndexedPack>();

    return true;
}

void ListModel::requestModVersions(ModPlatform::IndexedPack const& current,
                                   QModelIndex index)
{
    auto profile = (dynamic_cast<MinecraftInstance*>(
                        (dynamic_cast<ModPage*>(parent()))->m_instance))
                       ->getPackProfile();

    stopJob(m_versions_job.get(), this);

    m_versions_addon = current.addonId.toString();
    m_versions_index = index;
    m_versions_job = m_parent->apiProvider()->getVersions(
        {m_versions_addon, getMineVersions(), profile->getModLoaders(),
         m_parent->resourceType()},
        &m_versions_response);
    connect(m_versions_job.get(), &NetJob::succeeded, this,
            &ListModel::versionRequestSucceeded);
    m_versions_job->start();
}

void ListModel::performPaginatedSearch()
{
    auto profile = (dynamic_cast<MinecraftInstance*>(
                        (dynamic_cast<ModPage*>(parent()))->m_instance))
                       ->getPackProfile();

    stopJob(jobPtr.get(), this);

    jobPtr = m_parent->apiProvider()->searchMods(
        {nextSearchOffset, currentSearchTerm, getSorts()[currentSort],
         profile->getModLoaders(), getMineVersions(), m_parent->resourceType()},
        &m_search_response, debugName());
    connect(jobPtr.get(), &NetJob::succeeded, this,
            &ListModel::searchRequestSucceeded);
    connect(jobPtr.get(), &NetJob::failed, this,
            &ListModel::searchRequestFailed);
    connect(jobPtr.get(), &NetJob::aborted, this,
            &ListModel::searchRequestAborted);
    jobPtr->start();
}

void ListModel::requestModInfo(const ModPlatform::IndexedPack& current,
                               QModelIndex index)
{
    stopJob(m_info_job.get(), this);

    m_info_addon = current.addonId;
    m_info_index = index;
    m_info_job = m_parent->apiProvider()->getProject(current.addonId.toString(),
                                                     &m_info_response);
    connect(m_info_job.get(), &NetJob::succeeded, this,
            &ListModel::infoRequestSucceeded);
    m_info_job->start();
}

void ListModel::refresh()
{
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

void ListModel::searchWithTerm(const QString& term, const int sort,
                               const bool filter_changed)
{
    if (currentSearchTerm == term &&
        currentSearchTerm.isNull() == term.isNull() && currentSort == sort &&
        !filter_changed)
    {
        return;
    }

    currentSearchTerm = term;
    currentSort = sort;

    refresh();
}

/******** Request callbacks ********/

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

void ListModel::searchRequestSucceeded()
{
    QJsonParseError parse_error{};
    QJsonDocument doc =
        QJsonDocument::fromJson(m_search_response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response from " << debugName()
                   << " at " << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << m_search_response;
        return;
    }

    searchRequestFinished(doc);
}

void ListModel::searchRequestFinished(QJsonDocument& doc)
{
    jobPtr.reset();

    QList<ModPlatform::IndexedPack> newList;
    auto packs = documentToArray(doc);

    for (auto packRaw : std::as_const(packs))
    {
        auto packObj = packRaw.toObject();

        ModPlatform::IndexedPack pack;
        try
        {
            loadIndexedPack(pack, packObj);
            newList.append(pack);
        }
        catch (const JSONValidationError& e)
        {
            qWarning() << "Error while loading mod from "
                       << m_parent->debugName() << ": " << e.cause();
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

void ListModel::searchRequestFailed(QString /*reason*/)
{
    auto failed_action = jobPtr->getFailedActions().at(0);
    if (!failed_action->m_reply)
    {
        // Network error
        QMessageBox::critical(
            nullptr, tr("Error"),
            tr("A network error occurred. Could not load mods."));
    }
    else if (failed_action->m_reply &&
             failed_action->m_reply
                     ->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                     .toInt() == 409)
    {
        // 409 Gone, notify user to update
        QMessageBox::critical(
            nullptr, tr("Error"),
            //: %1 refers to the launcher itself
            QString("%1 %2")
                .arg(m_parent->displayName())
                .arg(tr("API version too old!\nPlease update %1!")
                         .arg(BuildConfig.LAUNCHER_NAME)));
    }
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

void ListModel::searchRequestAborted()
{
    jobPtr.reset();

    // refresh() aborts the running search to start a new one
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

void ListModel::infoRequestSucceeded()
{
    QJsonParseError parse_error{};
    QJsonDocument doc = QJsonDocument::fromJson(m_info_response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response for mod info at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << m_info_response;
        return;
    }

    auto& pack = m_parent->getCurrent();
    if (pack.addonId != m_info_addon)
    {
        return;
    }

    qDebug() << "Loading mod info";

    try
    {
        auto obj = Json::requireObject(doc);
        loadExtraPackInfo(pack, obj);
    }
    catch (const JSONValidationError& e)
    {
        qDebug() << doc;
        qWarning() << "Error while reading " << debugName()
                   << " mod info: " << e.cause();
    }

    // Check if the index is still valid for this mod or not
    if (pack.addonId == data(m_info_index, Qt::UserRole)
                            .value<ModPlatform::IndexedPack>()
                            .addonId)
    {
        // Cache info :^)
        QVariant new_pack;
        new_pack.setValue(pack);
        if (!setData(m_info_index, new_pack, Qt::UserRole))
        {
            qWarning() << "Failed to cache mod info!";
        }
    }

    m_parent->updateUi();
}

void ListModel::versionRequestSucceeded()
{
    QJsonParseError parse_error{};
    QJsonDocument doc =
        QJsonDocument::fromJson(m_versions_response, &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
    {
        qWarning() << "Error while parsing JSON response for "
                      "getting versions at "
                   << parse_error.offset
                   << " reason: " << parse_error.errorString();
        qWarning() << m_versions_response;
        return;
    }

    auto& current = m_parent->getCurrent();
    if (m_versions_addon != current.addonId)
    {
        return;
    }

    auto arr =
        doc.isObject() ? Json::ensureArray(doc.object(), "data") : doc.array();

    try
    {
        loadIndexedPackVersions(current, arr);
    }
    catch (const JSONValidationError& e)
    {
        qDebug() << doc;
        qWarning() << "Error while reading " << debugName()
                   << " mod version: " << e.cause();
    }

    // Cache info :^)
    // Only if the row still holds this mod, a new search may have put another
    // mod there meanwhile
    if (data(m_versions_index, Qt::UserRole)
            .value<ModPlatform::IndexedPack>()
            .addonId == current.addonId)
    {
        QVariant new_pack;
        new_pack.setValue(current);
        if (!setData(m_versions_index, new_pack, Qt::UserRole))
        {
            qWarning() << "Failed to cache mod versions!";
        }
    }

    m_parent->updateModVersions();
}

}  // namespace ModPlatform

/******** Helpers ********/

std::list<Version> ModPlatform::ListModel::getMineVersions() const
{
    return m_parent->getFilter()->versions;
}
