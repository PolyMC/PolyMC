#include "SequentialTask.h"

SequentialTask::SequentialTask(QObject *parent) : Task(parent) 
{
}

void SequentialTask::addTask(Task::Ptr task)
{
    m_queue.append(task);
}

void SequentialTask::executeTask()
{
    m_currentIndex = -1;
    startNext();
}

void SequentialTask::startNext()
{
    if (m_currentIndex != -1)
    {
        Task::Ptr previous = m_queue[m_currentIndex];
        disconnect(previous.get(), nullptr, this, nullptr);
    }
    m_currentIndex++;
    if (m_queue.isEmpty() || m_currentIndex >= m_queue.size())
    {
        emitSucceeded();
        return;
    }
    Task::Ptr next = m_queue[m_currentIndex];
    connect(next.get(), SIGNAL(failed(QString)), this, SLOT(subTaskFailed(QString)));
    connect(next.get(), SIGNAL(status(QString)), this, SLOT(subTaskStatus(QString)));
    connect(next.get(), SIGNAL(progress(qint64, qint64)), this, SLOT(subTaskProgress(qint64, qint64)));
    connect(next.get(), SIGNAL(succeeded()), this, SLOT(startNext()));
    next->start();
}

void SequentialTask::subTaskFailed(const QString &msg)
{
    emitFailed(msg);
}
void SequentialTask::subTaskStatus(const QString &msg)
{
    setStatus(msg);
}
void SequentialTask::subTaskProgress(qint64 current, qint64 total)
{
    if(total == 0)
    {
        setProgress(0, 100);
        return;
    }
    setProgress(current, total);
}
