#include "ModDependencyTask.h"
#include "modplatform/flame/FlameModIndex.h"
#include "modplatform/modrinth/ModrinthPackIndex.h"

#include <QEventLoop>
#include <QJsonDocument>

ModDependencyTask::ModDependencyTask(const QList<Root>& roots, const std::list<Version>& mcVersions,
                                     ModAPI::ModLoaderTypes loaders, ModAPI::ResourceType type,
                                     QObject* parent)
    : Task{parent}, m_roots{roots}, m_mcVersions{mcVersions}, m_loaders{loaders}, m_type{type} {}

auto ModDependencyTask::key(ModPlatform::Provider p, const QVariant& id) -> QString
{
    return QString("%1:%2").arg(p == ModPlatform::Provider::FLAME ? "flame" : "modrinth", id.toString());
}

void ModDependencyTask::executeTask() {
    QSet<QString> seen;

    for (auto& r : m_roots)
        seen.insert(key(r.prov, r.addonId));

    QList<Job> queue;
    for (auto& r : m_roots)
        for (auto& d : r.deps)
            if (d.type == ModPlatform::DependencyType::Required &&
                !seen.contains(key(r.prov, d.modId))) {
                seen.insert(key(r.prov, d.modId));
                queue.append({r.prov, d, 0});
            }

    int done = 0, total = queue.size();
    while (!queue.isEmpty()) {
        if (m_aborted) {
            emitAborted();
            return;
        }

        auto job = queue.takeFirst();
        if (job.depth > 10)
            continue;

        setStatus(tr("Resolving dependencies (%1 of %2)").arg(done + 1).arg(total));
        setProgress(done, total);

        // FIXME(crueter): versionId
        auto ver = apiFor(job.prov)->getLatestVersion(
            {job.dep.modId.toString(), m_mcVersions, m_loaders, m_type});

        if (!ver.fileId.isValid() || ver.downloadUrl.isEmpty()) {
            m_unresolved << job.dep.modId.toString();
            continue;
        }

        auto pack = fetchPack(job);
        if (pack.slug.isEmpty()) {
            m_unresolved << job.dep.modId.toString();
            continue;
        }

        m_resolved.append({pack, ver});

        // get immediate deps
        for (auto& d : ver.dependencies)
            if (d.type == ModPlatform::DependencyType::Required &&
                !seen.contains(key(job.prov, d.modId))) {
                seen.insert(key(job.prov, d.modId));
                queue.append({job.prov, d, job.depth + 1});
                ++total;
            }
        ++done;
    }
    emitSucceeded();
}

ModPlatform::IndexedPack ModDependencyTask::fetchPack(const Job& job)
{
    QEventLoop loop;
    ModPlatform::IndexedPack pack;
    pack.addonId = job.dep.modId;
    pack.provider = job.prov;

    auto* resp = new QByteArray;
    auto* netJob = apiFor(job.prov)->getProject(job.dep.modId.toString(), resp);

    QObject::connect(netJob, &NetJob::succeeded, [resp, &pack, prov = job.prov] {
        QJsonParseError err{};
        QJsonDocument doc = QJsonDocument::fromJson(*resp, &err);
        if (err.error != QJsonParseError::NoError)
            return;
        if (prov == ModPlatform::Provider::FLAME) {
            auto data = doc.object().value("data").toObject();
            FlameMod::loadIndexedPack(pack, data);
        } else {
            auto obj = doc.object();
            Modrinth::loadIndexedPack(pack, obj);
        }
    });
    QObject::connect(netJob, &NetJob::finished, [&loop] { loop.quit(); });

    netJob->start();
    loop.exec();
    return pack;
}
