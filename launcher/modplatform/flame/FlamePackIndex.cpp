#include "FlamePackIndex.h"

void Flame::loadIndexedPack(Flame::IndexedPack& pack, const nlohmann::json& obj)
{
    pack.addonId = obj["id"];
    pack.name = obj["name"].get<std::string>().c_str();
    pack.description = obj.value("summary", "").c_str();

    const auto& logo = obj["logo"];
    pack.logoName = logo["title"].get<std::string>().c_str();
    pack.logoUrl = logo["thumbnailUrl"].get<std::string>().c_str();

    const auto& authors = obj["authors"];
    for (const auto& author : authors) {
        Flame::ModpackAuthor packAuthor;
        packAuthor.name = author["name"].get<std::string>().c_str();
        packAuthor.url = author["url"].get<std::string>().c_str();
        pack.authors.append(packAuthor);
    }
    int defaultFileId = obj["mainFileId"];

    bool found = false;
    // check if there are some files before adding the pack
    const auto& files = obj["latestFiles"];
    for (const auto& file : files) {
        int id = file["id"];

        // NOTE: for now, ignore everything that's not the default...
        if (id != defaultFileId)
            continue;

        if (file["gameVersions"].empty())
            continue;

        found = true;
        break;
    }
    if (!found) {
        throw std::runtime_error("Pack with no good file, skipping: " + pack.name.toStdString());
    }

    loadIndexedInfo(pack, obj);
}

void Flame::loadIndexedInfo(IndexedPack& pack, const nlohmann::json& obj)
{
    const nlohmann::json& links_obj = obj["links"];
    nlohmann::json temp;

    temp = links_obj.value("websiteUrl", nlohmann::json());
    if (temp.is_string()) {
        pack.extra.websiteUrl = temp.get<std::string>().c_str();
        if (pack.extra.websiteUrl.endsWith('/')) {
            pack.extra.websiteUrl.chop(1);
        } else {
            pack.extra.websiteUrl = "";
        }
    } else {
        pack.extra.websiteUrl = "";
    }

    temp = links_obj.value("issuesUrl", nlohmann::json());
    if (temp.is_string()) {
        pack.extra.issuesUrl = temp.get<std::string>().c_str();
        if (pack.extra.issuesUrl.endsWith('/')) {
            pack.extra.issuesUrl.chop(1);
        } else {
            pack.extra.issuesUrl = "";
        }
    } else {
        pack.extra.issuesUrl = "";
    }

    temp = links_obj.value("sourceUrl", nlohmann::json());
    if (temp.is_string()) {
        pack.extra.sourceUrl = temp.get<std::string>().c_str();
        if (pack.extra.sourceUrl.endsWith('/')) {
            pack.extra.sourceUrl.chop(1);
        } else {
            pack.extra.sourceUrl = "";
        }
    } else {
        pack.extra.sourceUrl = "";
    }

    temp = links_obj.value("wikiUrl", nlohmann::json());
    if (temp.is_string()) {
        pack.extra.wikiUrl = temp.get<std::string>().c_str();
        if (pack.extra.wikiUrl.endsWith('/')) {
            pack.extra.wikiUrl.chop(1);
        } else {
            pack.extra.wikiUrl = "";
        }
    } else {
        pack.extra.wikiUrl = "";
    }

    pack.extraInfoLoaded = true;
}

void Flame::loadIndexedPackVersions(Flame::IndexedPack& pack, const nlohmann::json& arr)
{
    QVector<Flame::IndexedVersion> unsortedVersions;
    for (const auto& version : arr) {
        Flame::IndexedVersion file;

        file.addonId = pack.addonId;
        file.fileId = version["id"];
        const auto& versionArray = version["gameVersions"];
        if (versionArray.empty()) {
            continue;
        }

        // pick the latest version supported
        file.mcVersion = versionArray[0].get<std::string>().c_str();
        file.version = version["displayName"].get<std::string>().c_str();
        file.downloadUrl = version["downloadUrl"].get<std::string>().c_str();

        // only add if we have a download URL (third party distribution is enabled)
        if (!file.downloadUrl.isEmpty()) {
            unsortedVersions.append(file);
        }
    }

    auto orderSortPredicate = [](const IndexedVersion& a, const IndexedVersion& b) -> bool { return a.fileId > b.fileId; };
    std::sort(unsortedVersions.begin(), unsortedVersions.end(), orderSortPredicate);
    pack.versions = unsortedVersions;
    pack.versionsLoaded = true;
}
