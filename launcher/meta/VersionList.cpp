/* Copyright 2015-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "VersionList.h"

#include <QDebug>

#include <algorithm>
#include <functional>
#include <utility>

#include "Version.h"
#include "JsonFormat.h"

namespace Meta
{
namespace
{
bool isMinecraftRequire(const Require& req)
{
    return req.uid == "net.minecraft";
}

bool isNewerVersion(const VersionPtr& first, const VersionPtr& second)
{
    return first->rawTime() > second->rawTime();
}

bool isReleaseVersion(const VersionPtr& ptr)
{
    return ptr->type() == "release";
}
}  // namespace

VersionList::VersionList(const QString& uid, QObject* parent)
    : BaseVersionList(parent), m_uid(uid)
{
    setObjectName("Version list: " + uid);
}

Task::Ptr VersionList::getLoadTask()
{
    load(Net::Mode::Online);
    return getCurrentTask();
}

bool VersionList::isLoaded()
{
    return BaseEntity::isLoaded();
}

const BaseVersionPtr VersionList::at(int i) const
{
    return m_versions.at(i);
}
int VersionList::count() const
{
    return m_versions.size();
}

QVariant VersionList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 ||
        index.row() >= m_versions.size() || index.parent().isValid())
    {
        return QVariant();
    }

    VersionPtr version = m_versions.at(index.row());

    switch (role)
    {
        case VersionPointerRole:
            return QVariant::fromValue(
                std::dynamic_pointer_cast<BaseVersion>(version));
        case VersionRole:
        case VersionIdRole:
            return version->version();
        case ParentVersionRole:
        {
            // FIXME: HACK: this should be generic and be replaced by something
            // else. Anything that is a hard 'equals' dep is a 'parent uid'.
            auto& reqs = version->required();
            auto iter =
                std::find_if(reqs.begin(), reqs.end(), isMinecraftRequire);
            if (iter != reqs.end())
            {
                return (*iter).equalsVersion;
            }
            return QVariant();
        }
        case TypeRole:
            return version->type();

        case UidRole:
            return version->uid();
        case TimeRole:
            return version->time();
        case RequiresRole:
            return QVariant::fromValue(version->required());
        case SortRole:
            return version->rawTime();
        case VersionPtrRole:
            return QVariant::fromValue(version);
        case RecommendedRole:
            return version->isRecommended();
        // FIXME: this should be determined in whatever view/proxy is used...
        // case LatestRole: return version == getLatestStable();
        default:
            return QVariant();
    }
}

BaseVersionList::RoleList VersionList::providesRoles() const
{
    return {VersionPointerRole, VersionRole,  VersionIdRole,
            ParentVersionRole,  TypeRole,     UidRole,
            TimeRole,           RequiresRole, SortRole,
            RecommendedRole,    LatestRole,   VersionPtrRole};
}

QHash<int, QByteArray> VersionList::roleNames() const
{
    QHash<int, QByteArray> roles = BaseVersionList::roleNames();
    roles.insert(UidRole, "uid");
    roles.insert(TimeRole, "time");
    roles.insert(SortRole, "sort");
    roles.insert(RequiresRole, "requires");
    return roles;
}

QString VersionList::localFilename() const
{
    return m_uid + "/index.json";
}

QString VersionList::humanReadable() const
{
    return m_name.isEmpty() ? m_uid : m_name;
}

VersionPtr VersionList::getVersion(const QString& version)
{
    VersionPtr out = m_lookup.value(version, nullptr);
    if (!out)
    {
        out = std::make_shared<Version>(m_uid, version);
        m_lookup[version] = out;
    }
    return out;
}

bool VersionList::hasVersion(QString version) const
{
    for (const VersionPtr& entry : m_versions)
    {
        if (entry->version() == version)
        {
            return true;
        }
    }
    return false;
}

void VersionList::setName(const QString& name)
{
    m_name = name;
    emit nameChanged(name);
}

void VersionList::setVersions(const QVector<VersionPtr>& versions)
{
    beginResetModel();
    m_versions = versions;
    std::sort(m_versions.begin(), m_versions.end(), isNewerVersion);
    for (int i = 0; i < m_versions.size(); ++i)
    {
        m_lookup.insert(m_versions.at(i)->version(), m_versions.at(i));
        setupAddedVersion(i, m_versions.at(i));
    }

    // FIXME: this is dumb, we have 'recommended' as part of the metadata
    // already...
    auto recommendedIt = std::find_if(m_versions.constBegin(),
                                      m_versions.constEnd(), isReleaseVersion);
    m_recommended =
        recommendedIt == m_versions.constEnd() ? nullptr : *recommendedIt;
    endResetModel();
}

void VersionList::parse(const QJsonObject& obj)
{
    parseVersionList(obj, this);
}

// FIXME: this is dumb, we have 'recommended' as part of the metadata already...
static const Meta::VersionPtr& getBetterVersion(const Meta::VersionPtr& a,
                                                const Meta::VersionPtr& b)
{
    if (!a)
    {
        return b;
    }
    if (!b)
    {
        return a;
    }
    if (a->type() == b->type())
    {
        // newer of same type wins
        return (a->rawTime() > b->rawTime() ? a : b);
    }
    // 'release' type wins
    return (a->type() == "release" ? a : b);
}

void VersionList::mergeFromIndex(const VersionListPtr& other)
{
    if (m_name != other->m_name)
    {
        setName(other->m_name);
    }
    if (!other->m_sha256.isEmpty())
    {
        m_sha256 = other->m_sha256;
    }
}

void VersionList::merge(const VersionListPtr& other)
{
    if (m_name != other->m_name)
    {
        setName(other->m_name);
    }

    // TODO: do not reset the whole model. maybe?
    beginResetModel();
    m_versions.clear();
    if (other->m_versions.isEmpty())
    {
        qWarning() << "Empty list loaded ...";
    }
    for (VersionPtr version : std::as_const(other->m_versions))
    {
        // we already have the version. merge the contents, and keep using the
        // object everyone already has
        if (m_lookup.contains(version->version()))
        {
            auto existing = m_lookup.value(version->version());
            existing->mergeFromList(version);
            version = existing;
        }
        else
        {
            // keyed by version, like everywhere else (getVersion() looks it up
            // like that)
            m_lookup.insert(version->version(), version);
        }
        // connect it.
        setupAddedVersion(m_versions.size(), version);
        m_versions.append(version);
        m_recommended = getBetterVersion(m_recommended, version);
    }
    endResetModel();
}

void VersionList::setupAddedVersion(const int row, const VersionPtr& version)
{
    // FIXME: do not disconnect from everythin, disconnect only the lambdas here
    version->disconnect();
    connect(version.get(), &Version::requiresChanged, this,
            std::bind(&VersionList::versionRequiresChanged, this, row));
    connect(version.get(), &Version::timeChanged, this,
            std::bind(&VersionList::versionTimeChanged, this, row));
    connect(version.get(), &Version::typeChanged, this,
            std::bind(&VersionList::versionTypeChanged, this, row));
}

void VersionList::versionRequiresChanged(int row)
{
    emit dataChanged(index(row), index(row), QVector<int>() << RequiresRole);
}

void VersionList::versionTimeChanged(int row)
{
    emit dataChanged(index(row), index(row),
                     QVector<int>() << TimeRole << SortRole);
}

void VersionList::versionTypeChanged(int row)
{
    emit dataChanged(index(row), index(row), QVector<int>() << TypeRole);
}

BaseVersionPtr VersionList::getRecommended() const
{
    return m_recommended;
}

}  // namespace Meta

#include "moc_VersionList.cpp"
