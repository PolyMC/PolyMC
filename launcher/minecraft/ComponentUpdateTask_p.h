#pragma once

#include "net/Mode.h"
#include <QList>
#include <QString>
#include <cstddef>

class PackProfile;

struct RemoteLoadStatus
{
    enum class Type
    {
        Index,
        List,
        Version
    } type = Type::Version;
    size_t PackProfileIndex = 0;
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
    ComponentUpdateTask::Mode mode;
    Net::Mode netmode;
};
