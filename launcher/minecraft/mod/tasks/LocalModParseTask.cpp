#include "LocalModParseTask.h"

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <toml++/toml.h>
#include <QString>

#include "FileSystem.h"
#include <nlohmann/json.hpp>
#include "settings/INIFile.h"

namespace {

// NEW format
// https://github.com/MinecraftForge/FML/wiki/FML-mod-information-file/6f62b37cea040daf350dc253eae6326dd9c822c3

// OLD format:
// https://github.com/MinecraftForge/FML/wiki/FML-mod-information-file/5bf6a2d05145ec79387acc0d45c958642fb049fc
ModDetails ReadMCModInfo(const QByteArray& contents)
{
    auto getInfoFromArray = [&](nlohmann::json::array_t arr) -> ModDetails {
        if (!arr.at(0).is_object()) {
            return {};
        }
        ModDetails details;
        auto firstObj = arr.at(0);
        details.mod_id = firstObj.value("modid", "").c_str();
        QString name = firstObj.value("name", "").c_str();
        // NOTE: ignore stupid example mods copies where the author didn't even bother to change the name
        if (name != "Example Mod") {
            details.name = name;
        }
        details.version = firstObj.value("version", "").c_str();
        QString homeurl = firstObj.value("url", "").c_str();
        homeurl = homeurl.trimmed();

        if (!homeurl.isEmpty()) {
            // fix up url.
            if (!homeurl.startsWith("http://") && !homeurl.startsWith("https://") && !homeurl.startsWith("ftp://")) {
                homeurl.prepend("http://");
            }
        }
        details.homeurl = homeurl;
        details.description = firstObj.value("description", "").c_str();
        nlohmann::json::array_t authors = firstObj.value("authorList", nlohmann::json::array_t());
        if (authors.empty()) {
            // FIXME: what is the format of this? is there any?
            authors = firstObj.value("authors", nlohmann::json::array_t());
        }

        for (const auto& author : authors) {
            details.authors.append(author.get<std::string>().c_str());
        }
        return details;
    };

    nlohmann::json jsonDoc;
    try {
         jsonDoc = nlohmann::json::parse(contents.constData(), contents.constData() + contents.size());
    }
    catch (const nlohmann::json::parse_error& e) {
         qCritical() << "BAD stuff happened to mod json:";
         qCritical() << contents;
         qCritical() << e.what() << "id: " << e.id << "byte: " << e.byte;
         return {};
    }
    // this is the very old format that had just the array
    if (jsonDoc.is_array()) {
        return getInfoFromArray(jsonDoc);
    } else if (jsonDoc.is_object()) {
        auto val = jsonDoc.value("modinfoversion", nlohmann::json());
        if (val.is_null()) {
            val = jsonDoc.value("modListVersion", nlohmann::json());
        }

        int version = val.is_number_integer() ? val.get<int>() : -1;

        // Some mods set the number with "", so it's a String instead
        if (version < 0)
            version = std::stoi(val.get<std::string>());

        if (version != 2) {
            qCritical() << "BAD stuff happened to mod json:";
            qCritical() << contents;
            return {};
        }

        auto arrVal = jsonDoc.value("modlist", nlohmann::json());
        if (arrVal.is_null()) {
            arrVal = jsonDoc.value("modList", nlohmann::json());
        }
        if (arrVal.is_array()) {
            return getInfoFromArray(arrVal);
        }
    }
    return {};
}

// https://github.com/MinecraftForge/Documentation/blob/5ab4ba6cf9abc0ac4c0abd96ad187461aefd72af/docs/gettingstarted/structuring.md
ModDetails ReadMCModTOML(const QByteArray& contents)
{
    ModDetails details;

    toml::table tomlData;
#if TOML_EXCEPTIONS
    try {
        tomlData = toml::parse(contents.toStdString());
    } catch (const toml::parse_error& err) {
        return {};
    }
#else
    tomlData = toml::parse(contents.toStdString());
    if (!tomlData) {
        return {};
    }
#endif

    // array defined by [[mods]]
    auto tomlModsArr = tomlData["mods"].as_array();
    if (!tomlModsArr) {
        qWarning() << "Corrupted mods.toml? Couldn't find [[mods]] array!";
        return {};
    }

    // we only really care about the first element, since multiple mods in one file is not supported by us at the moment
    auto tomlModsTable0 = tomlModsArr->get(0);
    if (!tomlModsTable0) {
        qWarning() << "Corrupted mods.toml? [[mods]] didn't have an element at index 0!";
        return {};
    }
    auto modsTable = tomlModsTable0->as_table();

    // mandatory properties - always in [[mods]]
    if (auto modIdDatum = (*modsTable)["modId"].as_string()) {
        details.mod_id = QString::fromStdString(modIdDatum->get());
    }
    if (auto versionDatum = (*modsTable)["version"].as_string()) {
        details.version = QString::fromStdString(versionDatum->get());
    }
    if (auto displayNameDatum = (*modsTable)["displayName"].as_string()) {
        details.name = QString::fromStdString(displayNameDatum->get());
    }
    if (auto descriptionDatum = (*modsTable)["description"].as_string()) {
        details.description = QString::fromStdString(descriptionDatum->get());
    }

    // optional properties - can be in the root table or [[mods]]
    QString authors = "";
    auto authorsDatum = tomlData["authors"].as_string();
    if (!authorsDatum) {
        authorsDatum = (*modsTable)["authors"].as_string();
    }
    authors = authorsDatum ? QString::fromStdString(authorsDatum->get()) : "";
    if (!authors.isEmpty()) {
        details.authors.append(authors);
    }

    QString homeurl = "";
    auto homeurlDatum = tomlData["displayURL"].as_string();
    if (!homeurlDatum) {
        homeurlDatum = (*modsTable)["displayURL"].as_string();
    }
    homeurl = homeurlDatum ? QString::fromStdString(homeurlDatum->get()) : "";

    // fix up url.
    if (!homeurl.isEmpty() && !homeurl.startsWith("http://") && !homeurl.startsWith("https://") && !homeurl.startsWith("ftp://")) {
        homeurl.prepend("http://");
    }
    details.homeurl = homeurl;

    return details;
}

// https://fabricmc.net/wiki/documentation:fabric_mod_json
ModDetails ReadFabricModInfo(const QByteArray& contents)
{
    nlohmann::json jsonDoc;
    try {
        jsonDoc = nlohmann::json::parse(contents.constData(), contents.constData() + contents.size());
    } catch (const nlohmann::json::parse_error& err) {
        qWarning() << "Failed to parse fabric.mod.json: " << err.what() << " at " << err.byte << " in " << contents;
        return {};
    }
    ModDetails details;

    details.mod_id = jsonDoc.value("id", "").c_str();
    details.version = jsonDoc.value("version", "").c_str();
    details.name = jsonDoc.value("name", details.mod_id.toStdString()).c_str();
    details.description = jsonDoc.value("description", "").c_str();

    int schemaVersion = jsonDoc.value("schemaVersion", 0);
    if (schemaVersion >= 1) {
        const nlohmann::json::array_t& authors = jsonDoc.value("authors", nlohmann::json::array_t());
        for (const auto& author : authors) {
            if (author.is_object()) {
                details.authors.append(author.value("name", nlohmann::json()).get<std::string>().c_str());
            } else {
                details.authors.append(author.get<std::string>().c_str());
            }
        }

        if (jsonDoc.contains("contact")) {
            nlohmann::json contact = jsonDoc["contact"];

            details.homeurl = contact.value("homepage", "").c_str();
        }
    }
    return details;
}

// https://github.com/QuiltMC/rfcs/blob/master/specification/0002-quilt.mod.json.md
ModDetails ReadQuiltModInfo(const QByteArray& contents)
{
    const nlohmann::json& jsonDoc = nlohmann::json::parse(contents.constData(), contents.constData() + contents.size());

    ModDetails details;

    // https://github.com/QuiltMC/rfcs/blob/be6ba280d785395fefa90a43db48e5bfc1d15eb4/specification/0002-quilt.mod.json.
    int schemaVersion = jsonDoc.value("schema_version", 0);
    if (schemaVersion == 1) {
        const nlohmann::json& modInfo = jsonDoc.value("quilt_loader", nlohmann::json());

        details.mod_id = modInfo["id"].get<std::string>().c_str();
        details.version = modInfo["version"].get<std::string>().c_str();

        const nlohmann::json& modMetadata = modInfo.value("metadata", nlohmann::json());

        details.name = modMetadata.value("name", "").c_str();
        details.description = modMetadata.value("description", "").c_str();

        const nlohmann::json& modContributors = modMetadata.value("contributors", nlohmann::json());

        // We don't really care about the role of a contributor here
        QStringList keys;
        for (const auto& it: modContributors.items()) {
            keys.append(it.key().c_str());
        }
        details.authors += keys;

        const nlohmann::json& modContact = modMetadata.value("contact", nlohmann::json());

        if (modContact.contains("homepage")) {
            details.homeurl = modContact["homepage"].get<std::string>().c_str();
        }
    }
    return details;
}

ModDetails ReadForgeInfo(const QByteArray& contents)
{
    ModDetails details;
    // Read the data
    details.name = "Minecraft Forge";
    details.mod_id = "Forge";
    details.homeurl = "http://www.minecraftforge.net/forum/";
    INIFile ini;
    if (!ini.loadFile(contents))
        return details;

    QString major = ini.get("forge.major.number", "0").toString();
    QString minor = ini.get("forge.minor.number", "0").toString();
    QString revision = ini.get("forge.revision.number", "0").toString();
    QString build = ini.get("forge.build.number", "0").toString();

    details.version = major + "." + minor + "." + revision + "." + build;
    return details;
}

ModDetails ReadLiteModInfo(const QByteArray& contents)
{
    nlohmann::json jsonDoc = nlohmann::json::parse(contents.constData(), contents.constData() + contents.size());

    ModDetails details;

    if (jsonDoc.contains("name")) {
        details.mod_id = details.name = jsonDoc["name"].get<std::string>().c_str();
    }

    if (jsonDoc.contains("version")) {
        details.version = jsonDoc["version"].get<std::string>().c_str();
    } else {
        details.version = jsonDoc["revision"].get<std::string>().c_str();
    }
    details.mcversion = jsonDoc["mcversion"].get<std::string>().c_str();
    details.authors.append(jsonDoc.value("author", "").c_str());
    details.description = jsonDoc.value("description", "").c_str();
    details.homeurl = jsonDoc.value("url", "").c_str();
    return details;
}

}  // namespace

LocalModParseTask::LocalModParseTask(int token, ResourceType type, const QFileInfo& modFile)
    : Task(nullptr, false), m_token(token), m_type(type), m_modFile(modFile), m_result(new Result())
{}

void LocalModParseTask::processAsZip()
{
    QuaZip zip(m_modFile.filePath());
    if (!zip.open(QuaZip::mdUnzip))
        return;

    QuaZipFile file(&zip);

    if (zip.setCurrentFile("META-INF/mods.toml")) {
        if (!file.open(QIODevice::ReadOnly)) {
            zip.close();
            return;
        }

        m_result->details = ReadMCModTOML(file.readAll());
        file.close();

        // to replace ${file.jarVersion} with the actual version, as needed
        if (m_result->details.version == "${file.jarVersion}") {
            if (zip.setCurrentFile("META-INF/MANIFEST.MF")) {
                if (!file.open(QIODevice::ReadOnly)) {
                    zip.close();
                    return;
                }

                // quick and dirty line-by-line parser
                auto manifestLines = file.readAll().split('\n');
                QString manifestVersion = "";
                for (auto& line : manifestLines) {
                    if (QString(line).startsWith("Implementation-Version: ")) {
                        manifestVersion = QString(line).remove("Implementation-Version: ");
                        break;
                    }
                }

                // some mods use ${projectversion} in their build.gradle, causing this mess to show up in MANIFEST.MF
                // also keep with forge's behavior of setting the version to "NONE" if none is found
                if (manifestVersion.contains("task ':jar' property 'archiveVersion'") || manifestVersion == "") {
                    manifestVersion = "NONE";
                }

                m_result->details.version = manifestVersion;

                file.close();
            }
        }

        zip.close();
        return;
    } else if (zip.setCurrentFile("mcmod.info")) {
        if (!file.open(QIODevice::ReadOnly)) {
            zip.close();
            return;
        }

        m_result->details = ReadMCModInfo(file.readAll());
        file.close();
        zip.close();
        return;
    } else if (zip.setCurrentFile("quilt.mod.json")) {
        if (!file.open(QIODevice::ReadOnly)) {
            zip.close();
            return;
        }

        m_result->details = ReadQuiltModInfo(file.readAll());
        file.close();
        zip.close();
        return;
    } else if (zip.setCurrentFile("fabric.mod.json")) {
        if (!file.open(QIODevice::ReadOnly)) {
            zip.close();
            return;
        }

        m_result->details = ReadFabricModInfo(file.readAll());
        file.close();
        zip.close();
        return;
    } else if (zip.setCurrentFile("forgeversion.properties")) {
        if (!file.open(QIODevice::ReadOnly)) {
            zip.close();
            return;
        }

        m_result->details = ReadForgeInfo(file.readAll());
        file.close();
        zip.close();
        return;
    }

    zip.close();
}

void LocalModParseTask::processAsFolder()
{
    QFileInfo mcmod_info(FS::PathCombine(m_modFile.filePath(), "mcmod.info"));
    if (mcmod_info.isFile()) {
        QFile mcmod(mcmod_info.filePath());
        if (!mcmod.open(QIODevice::ReadOnly))
            return;
        auto data = mcmod.readAll();
        if (data.isEmpty() || data.isNull())
            return;
        m_result->details = ReadMCModInfo(data);
    }
}

void LocalModParseTask::processAsLitemod()
{
    QuaZip zip(m_modFile.filePath());
    if (!zip.open(QuaZip::mdUnzip))
        return;

    QuaZipFile file(&zip);

    if (zip.setCurrentFile("litemod.json")) {
        if (!file.open(QIODevice::ReadOnly)) {
            zip.close();
            return;
        }

        m_result->details = ReadLiteModInfo(file.readAll());
        file.close();
    }
    zip.close();
}

bool LocalModParseTask::abort()
{
    m_aborted.store(true);
    return true;
}

void LocalModParseTask::executeTask()
{
    switch (m_type) {
        case ResourceType::ZIPFILE:
            processAsZip();
            break;
        case ResourceType::FOLDER:
            processAsFolder();
            break;
        case ResourceType::LITEMOD:
            processAsLitemod();
            break;
        default:
            break;
    }

    if (m_aborted)
        emit finished();
    else
        emitSucceeded();
}
