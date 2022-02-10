#pragma once

#include <QWidget>

#include "modplatform/flame/FlameModIndex.h"
#include "tasks/Task.h"
#include "ui/pages/BasePage.h"
#include <Application.h>

namespace Ui
{
class FlameModPage;
}

class ModDownloadDialog;

namespace FlameMod {
    class ListModel;
}

class FlameModPage : public QWidget, public BasePage
{
    Q_OBJECT

public:
    explicit FlameModPage(ModDownloadDialog *dialog, BaseInstance *instance);
    ~FlameModPage() override;
    QString displayName() const override
    {
        return tr("CurseForge");
    }
    QIcon icon() const override
    {
        return APPLICATION->getThemedIcon("flame");
    }
    QString id() const override
    {
        return "curseforge";
    }
    QString helpPage() const override
    {
        return "Flame-platform";
    }
    bool shouldDisplay() const override;

    void openedImpl() override;

    bool eventFilter(QObject * watched, QEvent * event) override;

    BaseInstance *m_instance;

private:
    void suggestCurrent();

private slots:
    void triggerSearch();
    void onSelectionChanged(QModelIndex first, QModelIndex second);
    void onVersionSelectionChanged(QString data);

private:
    Ui::FlameModPage *ui = nullptr;
    ModDownloadDialog* dialog = nullptr;
    FlameMod::ListModel* listModel = nullptr;
    FlameMod::IndexedPack current;

    QString selectedVersion;
};
