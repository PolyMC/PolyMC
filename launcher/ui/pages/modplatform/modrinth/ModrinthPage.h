#pragma once

#include <QWidget>

#include "ui/pages/BasePage.h"
#include <Application.h>
#include "tasks/Task.h"
#include "modplatform/modrinth/ModrinthPackIndex.h"

namespace Ui
{
class ModrinthPage;
}

class ModDownloadDialog;

namespace Modrinth {
    class ListModel;
}

class ModrinthPage : public QWidget, public BasePage
{
    Q_OBJECT

public:
    explicit ModrinthPage(ModDownloadDialog *dialog, BaseInstance *instance);
    ~ModrinthPage() override;
    QString displayName() const override
    {
        return tr("Modrinth");
    }
    QIcon icon() const override
    {
        return APPLICATION->getThemedIcon("modrinth");
    }
    QString id() const override
    {
        return "modrinth";
    }
    QString helpPage() const override
    {
        return "Modrinth-platform";
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
    Ui::ModrinthPage *ui = nullptr;
    ModDownloadDialog* dialog = nullptr;
    Modrinth::ListModel* listModel = nullptr;
    Modrinth::IndexedPack current;

    QString selectedVersion;
};
