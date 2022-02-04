#pragma once

#include "ModFolderPage.h"
#include "ui_ModFolderPage.h"

class ShaderPackPage : public ModFolderPage
{
    Q_OBJECT
public:
    explicit ShaderPackPage(MinecraftInstance *instance, QWidget *parent = nullptr)
        : ModFolderPage(instance, instance->shaderPackList(), "shaderpacks",
                        "shaderpacks", tr("Shader packs"), "Resource-packs", parent)
    {
        ui->actionView_configs->setVisible(false);
    }
    ~ShaderPackPage() override = default;

    bool shouldDisplay() const override
    {
        return true;
    }
};
