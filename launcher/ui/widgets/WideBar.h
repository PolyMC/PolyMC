#pragma once

#include <QAction>
#include <QMap>
#include <QToolBar>

class QMenu;

class WideBar : public QToolBar {
    Q_OBJECT

   public:
    explicit WideBar(const QString& title, QWidget* parent = nullptr);
    explicit WideBar(QWidget* parent = nullptr);
    virtual ~WideBar();

    void addAction(QAction* action);
    void addSeparator();

    void insertSpacer(QAction* action);
    void insertSeparator(QAction* before);
    void insertActionBefore(QAction* before, QAction* action);
    void insertActionAfter(QAction* after, QAction* action);

    QMenu* createContextMenu(QWidget* parent = nullptr, const QString& title = QString());

   private:
    struct BarEntry;

    QList<BarEntry*>::iterator getMatching(QAction* act);

   private:
    QList<BarEntry*> m_entries;
};
