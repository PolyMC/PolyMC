//
// Created by timoreo on 16/01/2022.
//

#pragma once

#include "modplatform/ModIndex.h"

#include "BaseInstance.h"
#include <QNetworkAccessManager>

namespace FlameMod {

void loadIndexedPack(ModPlatform::IndexedPack& m, const nlohmann::json& obj);
void loadURLs(ModPlatform::IndexedPack& m, const nlohmann::json& obj);
void loadBody(ModPlatform::IndexedPack& m, const nlohmann::json& obj);
void loadIndexedPackVersions(ModPlatform::IndexedPack& pack,
                             const nlohmann::json& arr,
                             const shared_qobject_ptr<QNetworkAccessManager>& network,
                             BaseInstance* inst);
auto loadIndexedPackVersion(const nlohmann::json& obj, bool load_changelog = false) -> ModPlatform::IndexedVersion;

}  // namespace FlameMod
