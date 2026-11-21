#include <QTest>

#include <tasks/ConcurrentTask.h>
#include <tasks/MultipleOptionsTask.h>
#include <tasks/SequentialTask.h>
#include <tasks/Task.h>

#include <QList>

#include <functional>

/* Does nothing. Only used for testing. */
class BasicTask : public Task
{
    Q_OBJECT

    friend class TaskTest;

private:
    void executeTask() override
    {
        emitSucceeded();
    };
};

/* Does nothing. Only used for testing. */
class BasicTask_MultiStep : public Task
{
    Q_OBJECT

    friend class TaskTest;

private:
    bool isMultiStep() const override
    {
        return true;
    }

    void executeTask() override {};
};

namespace
{
void verifyFinishedTasks(const Task& task, const QList<const Task*>& succeeded,
                         const QList<const Task*>& failed)
{
    QVERIFY2(task.wasSuccessful(),
             "Task finished but was not successful "
             "when it should have been.");
    for (auto subtask : succeeded)
    {
        QVERIFY(subtask->wasSuccessful());
    }
    for (auto subtask : failed)
    {
        QVERIFY(!subtask->wasSuccessful());
    }
}

bool isTaskFinished(const Task& task)
{
    return task.isFinished();
}
}  // namespace

class TaskTest : public QObject
{
    Q_OBJECT

private slots:
    void test_SetStatus_NoMultiStep()
    {
        BasicTask t;
        QString status{"test status"};

        t.setStatus(status);

        QCOMPARE(t.getStatus(), status);
        QCOMPARE(t.getStepStatus(), status);
    }

    void test_SetStatus_MultiStep()
    {
        BasicTask_MultiStep t;
        QString status{"test status"};

        t.setStatus(status);

        QCOMPARE(t.getStatus(), status);
        // Even though it is multi step, it does not override the getStepStatus
        // method, so it should remain the same.
        QCOMPARE(t.getStepStatus(), status);
    }

    void test_SetProgress()
    {
        BasicTask t;
        int current = 42;
        int total = 207;

        t.setProgress(current, total);

        QCOMPARE(t.getProgress(), current);
        QCOMPARE(t.getTotalProgress(), total);
    }

    void test_basicRun()
    {
        BasicTask t;
        QObject::connect(&t, &Task::finished,
                         std::bind(&verifyFinishedTasks, std::cref(t),
                                   QList<const Task*>{}, QList<const Task*>{}));
        t.start();

        QVERIFY2(
            QTest::qWaitFor(std::bind(&isTaskFinished, std::cref(t)), 1000),
            "Task didn't finish as it should.");
    }

    void test_basicConcurrentRun()
    {
        BasicTask t1;
        BasicTask t2;
        BasicTask t3;

        ConcurrentTask t;

        t.addTask(&t1);
        t.addTask(&t2);
        t.addTask(&t3);

        QObject::connect(
            &t, &Task::finished,
            std::bind(&verifyFinishedTasks, std::cref(t),
                      QList<const Task*>{&t1, &t2, &t3}, QList<const Task*>{}));

        t.start();
        QVERIFY2(
            QTest::qWaitFor(std::bind(&isTaskFinished, std::cref(t)), 1000),
            "Task didn't finish as it should.");
    }

    // Tests if starting new tasks after the 6 initial ones is working
    void test_moreConcurrentRun()
    {
        BasicTask t1, t2, t3, t4, t5, t6, t7, t8, t9;

        ConcurrentTask t;

        t.addTask(&t1);
        t.addTask(&t2);
        t.addTask(&t3);
        t.addTask(&t4);
        t.addTask(&t5);
        t.addTask(&t6);
        t.addTask(&t7);
        t.addTask(&t8);
        t.addTask(&t9);

        QObject::connect(&t, &Task::finished,
                         std::bind(&verifyFinishedTasks, std::cref(t),
                                   QList<const Task*>{&t1, &t2, &t3, &t4, &t5,
                                                      &t6, &t7, &t8, &t9},
                                   QList<const Task*>{}));

        t.start();
        QVERIFY2(
            QTest::qWaitFor(std::bind(&isTaskFinished, std::cref(t)), 1000),
            "Task didn't finish as it should.");
    }

    void test_basicSequentialRun()
    {
        BasicTask t1;
        BasicTask t2;
        BasicTask t3;

        SequentialTask t;

        t.addTask(&t1);
        t.addTask(&t2);
        t.addTask(&t3);

        QObject::connect(
            &t, &Task::finished,
            std::bind(&verifyFinishedTasks, std::cref(t),
                      QList<const Task*>{&t1, &t2, &t3}, QList<const Task*>{}));

        t.start();
        QVERIFY2(
            QTest::qWaitFor(std::bind(&isTaskFinished, std::cref(t)), 1000),
            "Task didn't finish as it should.");
    }

    void test_basicMultipleOptionsRun()
    {
        BasicTask t1;
        BasicTask t2;
        BasicTask t3;

        MultipleOptionsTask t;

        t.addTask(&t1);
        t.addTask(&t2);
        t.addTask(&t3);

        QObject::connect(
            &t, &Task::finished,
            std::bind(&verifyFinishedTasks, std::cref(t),
                      QList<const Task*>{&t1}, QList<const Task*>{&t2, &t3}));

        t.start();
        QVERIFY2(
            QTest::qWaitFor(std::bind(&isTaskFinished, std::cref(t)), 1000),
            "Task didn't finish as it should.");
    }
};

QTEST_GUILESS_MAIN(TaskTest)

#include "Task_test.moc"
