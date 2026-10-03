#pragma once

#include "FlameModPage.h"

namespace FlameMod {

class ListModel : public ModPlatform::ListModel {
    Q_OBJECT

   public:
    ListModel(FlameModPage* parent) : ModPlatform::ListModel(parent) {}
    ~ListModel() override = default;

   private:
    void loadIndexedPack(ModPlatform::IndexedPack& m, QJsonObject& obj) override;
    void loadExtraPackInfo(ModPlatform::IndexedPack& m, QJsonObject& obj) override;
    void loadIndexedPackVersions(ModPlatform::IndexedPack& m, QJsonArray& arr) override;

    QJsonArray documentToArray(QJsonDocument& obj) const override;

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    static const char* sorts[6]; 
    inline const char** getSorts() const override { return sorts; };
};

}  // namespace FlameMod
