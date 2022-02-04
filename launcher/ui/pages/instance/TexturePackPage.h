#pragma once

#include "ModFolderPage.h"
#include "ui_ModFolderPage.h"

class TexturePackPage : public ModFolderPage
{
    Q_OBJECT
public:
    explicit TexturePackPage(MinecraftInstance *instance, QWidget *parent = nullptr)
        : ModFolderPage(instance, instance->texturePackList(), "texturepacks", "resourcepacks",
                        tr("Texture packs"), "Texture-packs", parent)
    {
        ui->actionView_configs->setVisible(false);
    }
    ~TexturePackPage() override {}

    bool shouldDisplay() const override
    {
        return m_inst->traits().contains("texturepacks");
    }
};
