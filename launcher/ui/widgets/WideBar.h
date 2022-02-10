#pragma once

#include <QAction>
#include <QMap>
#include <QToolBar>

class QMenu;

class WideBar : public QToolBar
{
    Q_OBJECT

public:
    explicit WideBar(const QString &title, QWidget * parent = nullptr);
    explicit WideBar(QWidget * parent = nullptr);
    ~WideBar() override;

    void addAction(QAction *action);
    void addSeparator();
    void insertSpacer(QAction *action);
    void insertActionBefore(QAction *before, QAction *action);
    QMenu *createContextMenu(QWidget *parent = nullptr, const QString & title = QString());

private:
    struct BarEntry;
    QList<BarEntry *> m_entries;
};
