#pragma once

#include "ConcurrentTask.h"

/** A concurrent task that only allows one concurrent task :)
 *
 *  This should be used when there's a need to maintain a strict ordering of task executions, and
 *  the starting of a task is contingent on the success of the previous one.
 *
 *  See MultipleOptionsTask if that's not the case.
 */
class SequentialTask : public ConcurrentTask {
    Q_OBJECT
   public:
    explicit SequentialTask(QObject* parent = nullptr, const QString& task_name = "") : ConcurrentTask(parent, task_name, 1) {}
    ~SequentialTask() override = default;

   protected:
    void startNext() override;
    void updateState() override;
};
