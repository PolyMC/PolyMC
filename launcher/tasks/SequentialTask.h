#pragma once

#include "Task.h"
#include "QObjectPtr.h"

#include <QQueue>

class SequentialTask : public Task
{
    Q_OBJECT
public:
    explicit SequentialTask(QObject *parent = nullptr);
    ~SequentialTask() override {};

    void addTask(Task::Ptr task);

protected:
    void executeTask() override;

private
slots:
    void startNext();
    void subTaskFailed(const QString &msg);
    void subTaskStatus(const QString &msg);
    void subTaskProgress(qint64 current, qint64 total);

private:
    QQueue<Task::Ptr > m_queue;
    int m_currentIndex;
};
