#include "FlameModIndex.h"

#include "Json.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "modplatform/flame/FlameAPI.h"

static FlameAPI api;
static ModPlatform::ProviderCapabilities ProviderCaps;

void FlameMod::loadIndexedPack(ModPlatform::IndexedPack& pack, nlohmann::json& obj)
{
        pack.addonId = obj["id"].get<int>();
        pack.provider = ModPlatform::Provider::FLAME;
        pack.name = obj["name"].get<std::string>().c_str();
        pack.slug = obj["slug"].get<std::string>().c_str();

        nlohmann::json linksobj = obj.value("links", nlohmann::json::object());
        pack.websiteUrl = linksobj.value("websiteUrl", "").c_str();

        pack.description = obj.value("summary", "").c_str();

        nlohmann::json logo = obj.value("logo", nlohmann::json::object());

        pack.logoName = logo.value("title", "").c_str();
        pack.logoUrl = logo.value("thumbnailUrl", "").c_str();

        nlohmann::json authors = obj.value("authors", nlohmann::json::array());
        for (auto author : authors) {
            ModPlatform::ModpackAuthor packAuthor;
            packAuthor.name = author["name"].get<std::string>().c_str();
            packAuthor.url = author["url"].get<std::string>().c_str();
            pack.authors.append(packAuthor);
        }

        pack.extraDataLoaded = false;
        loadURLs(pack, linksobj);
}

void FlameMod::loadURLs(ModPlatform::IndexedPack& pack, nlohmann::json& obj)
{
    nlohmann::json temp;

    temp = obj.value("issuesUrl", nlohmann::json());
    if (!temp.is_null())
        pack.extraData.issuesUrl = temp.get<std::string>().c_str();
    if (pack.extraData.issuesUrl.endsWith('/'))
        pack.extraData.issuesUrl.chop(1);

    temp = obj.value("sourceUrl", nlohmann::json());
    if (!temp.is_null())
        pack.extraData.sourceUrl = temp.get<std::string>().c_str();
    if (pack.extraData.sourceUrl.endsWith('/'))
        pack.extraData.sourceUrl.chop(1);

    temp = obj.value("wikiUrl", nlohmann::json());
    if (!temp.is_null())
        pack.extraData.wikiUrl = temp.get<std::string>().c_str();
    if (pack.extraData.wikiUrl.endsWith('/'))
        pack.extraData.wikiUrl.chop(1);

    if (!pack.extraData.body.isEmpty())
        pack.extraDataLoaded = true;
}

void FlameMod::loadBody(ModPlatform::IndexedPack& pack, nlohmann::json& obj)
{
    pack.extraData.body  = api.getModDescription(pack.addonId.toInt());

    if (!pack.extraData.issuesUrl.isEmpty() || !pack.extraData.sourceUrl.isEmpty() || !pack.extraData.wikiUrl.isEmpty())
        pack.extraDataLoaded = true;
}

static QString enumToString(int hash_algorithm)
{
    switch(hash_algorithm){
    default:
    case 1:
        return "sha1";
    case 2:
        return "md5";
    }
}

void FlameMod::loadIndexedPackVersions(ModPlatform::IndexedPack& pack,
                                       nlohmann::json& arr,
                                       const shared_qobject_ptr<QNetworkAccessManager>& network,
                                       BaseInstance* inst)
{
    QVector<ModPlatform::IndexedVersion> unsortedVersions;
    auto profile = (dynamic_cast<MinecraftInstance*>(inst))->getPackProfile();
    QString mcVersion = profile->getComponentVersion("net.minecraft");

    for (const auto& obj : arr) {
        auto file = loadIndexedPackVersion(obj);
        if(!file.addonId.isValid())
            file.addonId = pack.addonId;

        if(file.fileId.isValid()) // Heuristic to check if the returned value is valid
            unsortedVersions.append(file);
    }

    auto orderSortPredicate = [](const ModPlatform::IndexedVersion& a, const ModPlatform::IndexedVersion& b) -> bool {
        // dates are in RFC 3339 format
        return a.date > b.date;
    };
    std::sort(unsortedVersions.begin(), unsortedVersions.end(), orderSortPredicate);
    pack.versions = unsortedVersions;
    pack.versionsLoaded = true;
}

auto FlameMod::loadIndexedPackVersion(const nlohmann::json& obj, bool load_changelog) -> ModPlatform::IndexedVersion
{
    auto versionArray = obj.value("gameVersions", nlohmann::json());
    if (versionArray.empty()) {
        return {};
    }

    ModPlatform::IndexedVersion file;

    for (const auto& mcVer : versionArray) {
        auto str = mcVer.get<std::string>();

        if (str.find('.') != std::string::npos)
            file.mcVersion.append(str.c_str());
    }
    file.addonId = obj["modId"].get<int>();
    file.fileId = obj["id"].get<int>();
    file.date = obj["fileDate"].get<std::string>().c_str();
    file.version = obj["displayName"].get<std::string>().c_str();
    file.downloadUrl = obj["downloadUrl"].get<std::string>().c_str();
    file.fileName = obj["fileName"].get<std::string>().c_str();

    auto hash_list = obj.value("hashes", nlohmann::json());
    for (const auto& hash_entry : hash_list) {
        auto hash_types = ProviderCaps.hashType(ModPlatform::Provider::FLAME);
        //auto hash_algo = enumToString(Json::ensureInteger(hash_entry, "algo", 1, "algorithm"));
        auto hash_algo = enumToString(hash_entry.value("algo", 1));
        if (hash_types.contains(hash_algo)) {
            file.hash = hash_entry.value("value", "").c_str();
            file.hash_type = hash_algo;
            break;
        }
    }

    if(load_changelog)
        file.changelog = api.getModFileChangelog(file.addonId.toInt(), file.fileId.toInt());

    return file;
}
