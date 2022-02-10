#include "LanguageWizardPage.h"
#include <Application.h>
#include <translations/TranslationsModel.h>

#include "ui/widgets/LanguageSelectionWidget.h"
#include <BuildConfig.h>
#include <QVBoxLayout>

LanguageWizardPage::LanguageWizardPage(QWidget *parent)
    : BaseWizardPage(parent)
{
    setObjectName(QStringLiteral("languagePage"));
    auto layout = new QVBoxLayout(this);
    mainWidget = new LanguageSelectionWidget(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(mainWidget);

    retranslate();
}

LanguageWizardPage::~LanguageWizardPage() = default;

bool LanguageWizardPage::wantsRefreshButton()
{
    return true;
}

void LanguageWizardPage::refresh()
{
    auto translations = APPLICATION->translations();
    translations->downloadIndex();
}

bool LanguageWizardPage::validatePage()
{
    auto settings = APPLICATION->settings();
    QString key = mainWidget->getSelectedLanguageKey();
    settings->set("Language", key);
    return true;
}

void LanguageWizardPage::retranslate()
{
    setTitle(tr("Language"));
    setSubTitle(tr("Select the language to use in %1").arg(BuildConfig.LAUNCHER_NAME));
    mainWidget->retranslate();
}
