#pragma once

#include "tasks/Task.h"

class MinecraftInstance;
class FoldersTask : public Task
{
    Q_OBJECT
public:
    explicit FoldersTask(MinecraftInstance * inst);
    virtual ~FoldersTask() {};

    void executeTask() override;
private:
    MinecraftInstance *m_inst;
};

