#pragma once

#include <QWidget>

#include "Application.h"
#include "modplatform/ModAPI.h"
#include "modplatform/ModIndex.h"
#include "ui/pages/BasePage.h"
#include "ui/pages/modplatform/ModModel.h"
#include "ui/widgets/ModFilterWidget.h"
#include "ui/widgets/ProgressWidget.h"

class ModDownloadDialog;

namespace Ui {
class ModPage;
}

/* This page handles most logic related to browsing and selecting mods to download. */
class ModPage : public QWidget, public BasePage {
    Q_OBJECT

   public:
    template<typename T>
    static T* create(ModDownloadDialog* dialog, ModAPI::ResourceType type, BaseInstance* instance)
    {
        auto page = new T(dialog, type, instance);

        auto filter_widget = ModFilterWidget::create(static_cast<MinecraftInstance*>(instance)->getPackProfile()->getComponentVersion("net.minecraft"), page);
        page->setFilterWidget(filter_widget);

        return page;
    }

    ~ModPage() override;

    /* Affects what the user sees */
    QString displayName() const override = 0;
    QIcon icon() const override = 0;
    QString id() const override = 0;
    QString helpPage() const override = 0;

    /* Used internally */
    virtual QString metaEntryBase() const = 0;
    virtual QString debugName() const = 0;


    void retranslate() override;

    void updateUi();

    bool shouldDisplay() const override = 0;
    virtual bool validateVersion(ModPlatform::IndexedVersion& ver, QString mineVer, ModAPI::ModLoaderTypes loaders = ModAPI::Unspecified) const = 0;
    virtual bool optedOut(ModPlatform::IndexedVersion& ver) const { return false; };

    ModAPI* apiProvider() { return api.get(); };
    const std::shared_ptr<ModFilterWidget::Filter> getFilter() const { return m_filter; }
    const ModDownloadDialog* getDialog() const { return dialog; }
    ModAPI::ResourceType resourceType() const { return m_resourceType; }

    /** Get the current term in the search bar. */
    QString getSearchTerm() const;
    /** Programatically set the term in the search bar. */
    void setSearchTerm(QString);

    void setFilterWidget(unique_qobject_ptr<ModFilterWidget>&);

    ModPlatform::IndexedPack& getCurrent() { return current; }
    void updateModVersions(int prev_count = -1);

    void openedImpl() override;
    bool eventFilter(QObject* watched, QEvent* event) override;

    BaseInstance* m_instance;

   protected:
    ModPage(ModDownloadDialog* dialog, ModAPI::ResourceType type, BaseInstance* instance, ModAPI* api);
    void updateSelectionButton();

   protected slots:
    virtual void filterMods();
    void triggerSearch();
    void onSelectionChanged(QModelIndex first, QModelIndex second);
    void onVersionSelectionChanged(QString data);
    void onModSelected();

   protected:
    Ui::ModPage* ui = nullptr;
    ModDownloadDialog* dialog = nullptr;

    unique_qobject_ptr<ModFilterWidget> m_filter_widget;
    std::shared_ptr<ModFilterWidget::Filter> m_filter;

    ProgressWidget m_fetch_progress;

    ModPlatform::ListModel* listModel = nullptr;
    ModPlatform::IndexedPack current;

    std::unique_ptr<ModAPI> api;

    ModAPI::ResourceType m_resourceType;
    QString m_typeString;

    int selectedVersion = -1;

    // Used to do instant searching with a delay to cache quick changes
    QTimer m_search_timer;
};
