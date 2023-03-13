#include "PackManifest.h"

#include <fstream>

static void loadFileV1(Flame::File& f, const nlohmann::json& file)
{
    f.projectId = file["projectID"];
    f.fileId = file["fileID"];
    f.required = file.value("required", true);

}

static void loadModloaderV1(Flame::Modloader& m, const nlohmann::json& modLoader)
{
    m.id = modLoader["id"].get<std::string>().c_str();
    m.primary = modLoader.value("primary", false);
}

static void loadMinecraftV1(Flame::Minecraft& m, const nlohmann::json& minecraft)
{
    m.version = minecraft["version"].get<std::string>().c_str();
    // extra libraries... apparently only used for a custom Minecraft launcher in the 1.2.5 FTB retro pack
    // intended use is likely hardcoded in the 'Flame' client, the manifest says nothing
    m.libraries = minecraft.value("libraries", "").c_str();
    //auto arr = Json::ensureArray(minecraft, "modLoaders", QJsonArray());
    auto arr = minecraft.value("modLoaders", nlohmann::json());
    for (const auto& obj : arr) {
        Flame::Modloader loader;
        loadModloaderV1(loader, obj);
        m.modLoaders.append(loader);
    }
}

static void loadManifestV1(Flame::Manifest& pack, const nlohmann::json& manifest)
{
    loadMinecraftV1(pack.minecraft, manifest["minecraft"]);

    pack.name = manifest.value("name", "Unnamed").c_str();
    pack.version = manifest.value("version", "").c_str();
    pack.author = manifest.value("author", "Anonymous").c_str();


    auto arr = manifest.value("files", nlohmann::json());
    for (const auto& obj : arr) {
        Flame::File file;
        loadFileV1(file, obj);

        pack.files.insert(file.fileId,file);
    }

    pack.overrides = manifest.value("overrides", "overrides").c_str();

    pack.is_loaded = true;
}

void Flame::loadManifest(Flame::Manifest& m, const QString& filepath)
{
    nlohmann::json obj = nlohmann::json::parse(std::ifstream(filepath.toStdString()));

    m.manifestType = obj["manifestType"].get<std::string>().c_str();
    if (m.manifestType != "minecraftModpack") {
        throw std::runtime_error("Not a modpack manifest");
    }
    m.manifestVersion = obj["manifestVersion"].get<int>();
    if (m.manifestVersion != 1) {
        throw std::runtime_error("Unknown manifest version" + std::to_string(m.manifestVersion));
    }
    loadManifestV1(m, obj);
}

bool Flame::File::parseFromObject(const nlohmann::json& obj,  bool throw_on_blocked)
{
    fileName = obj["fileName"].get<std::string>().c_str();
    // This is a piece of a Flame project JSON pulled out into the file metadata (here) for convenience
    // It is also optional
    type = File::Type::SingleFile;

    if (fileName.endsWith(".zip")) {
        // this is probably a resource pack
        targetFolder = "resourcepacks";
    } else {
        // this is probably a mod, dunno what else could modpacks download
        targetFolder = "mods";
    }
    // get the hash
    hash = QString();
    for(const auto& hash_obj : obj["hashes"]) {
        if (hash_obj["algo"].get<int>() == 1) {
            hash = hash_obj["value"].get<std::string>().c_str();;
        }
    }


    // may throw, if the project is blocked
    QString rawUrl = obj.value("downloadUrl", "").c_str();
    url = QUrl(rawUrl, QUrl::TolerantMode);
    if (!url.isValid() && throw_on_blocked) {
        throw std::runtime_error("Invalid URL: " + rawUrl.toStdString());
    }

    resolved = true;
    return true;
}
