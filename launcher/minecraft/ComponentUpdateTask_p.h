#pragma once

#include <cstddef>
#include <QString>
#include <QList>
#include "net/Mode.h"

class PackProfile;

struct RemoteLoadStatus
{
    enum class Type
    {
        Index,
        List,
        Version
    } type = Type::Version;
    QString uid;
    bool finished = false;
    bool succeeded = false;
    QString error;
};

struct ComponentUpdateTaskData
{
    PackProfile * m_list = nullptr;
    QList<RemoteLoadStatus> remoteLoadStatusList;
    bool remoteLoadSuccessful = true;
    size_t remoteTasksInProgress = 0;
    // How many times dependency resolution had to start over, to stop it from going on forever
    int resolutionRounds = 0;
    ComponentUpdateTask::Mode mode;
    Net::Mode netmode;
};
