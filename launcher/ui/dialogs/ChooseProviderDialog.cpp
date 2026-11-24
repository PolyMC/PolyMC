#include "ChooseProviderDialog.h"
#include "ui_ChooseProviderDialog.h"

#include <QPushButton>
#include <QRadioButton>

static ModPlatform::ProviderCapabilities ChooserProviderCaps;

ChooseProviderDialog::ChooseProviderDialog(QWidget* parent, bool single_choice,
                                           bool allow_skipping)
    : QDialog(parent), ui(new Ui::ChooseProviderDialog)
{
    ui->setupUi(this);

    addProviders();
    m_providers.button(0)->click();

    connect(ui->skipOneButton, &QPushButton::clicked, this,
            &ChooseProviderDialog::skipOne);
    connect(ui->skipAllButton, &QPushButton::clicked, this,
            &ChooseProviderDialog::skipAll);

    connect(ui->confirmOneButton, &QPushButton::clicked, this,
            &ChooseProviderDialog::confirmOne);
    connect(ui->confirmAllButton, &QPushButton::clicked, this,
            &ChooseProviderDialog::confirmAll);

    if (single_choice)
    {
        ui->providersLayout->removeWidget(ui->skipAllButton);
        ui->providersLayout->removeWidget(ui->confirmAllButton);
    }

    if (!allow_skipping)
    {
        ui->providersLayout->removeWidget(ui->skipOneButton);
        ui->providersLayout->removeWidget(ui->skipAllButton);
    }
}

ChooseProviderDialog::~ChooseProviderDialog()
{
    delete ui;
}

void ChooseProviderDialog::setDescription(QString desc)
{
    ui->explanationLabel->setText(desc);
}

void ChooseProviderDialog::skipOne()
{
    reject();
}
void ChooseProviderDialog::skipAll()
{
    m_response.skip_all = true;
    reject();
}

void ChooseProviderDialog::confirmOne()
{
    auto choice = getSelectedChoice();
    m_response.chosen = choice.provider;
    m_response.source = choice.source;
    m_response.try_others = ui->tryOthersCheckbox->isChecked();
    accept();
}
void ChooseProviderDialog::confirmAll()
{
    auto choice = getSelectedChoice();
    m_response.chosen = choice.provider;
    m_response.source = choice.source;
    m_response.confirm_all = true;
    m_response.try_others = ui->tryOthersCheckbox->isChecked();
    accept();
}

ChooseProviderDialog::Choice ChooseProviderDialog::getSelectedChoice() const
{
    return m_choices.value(m_providers.checkedId(),
                           {ModPlatform::Provider::MODRINTH, {}});
}

void ChooseProviderDialog::addProviders()
{
    m_choices.append({ModPlatform::Provider::MODRINTH, {}});
    m_choices.append({ModPlatform::Provider::FLAME, {}});
    for (const auto& source : Modrinth::customSources())
    {
        m_choices.append({ModPlatform::Provider::MODRINTH, source});
    }

    for (int i = 0; i < m_choices.size(); i++)
    {
        const auto& choice = m_choices.at(i);
        auto name = choice.source.isOfficial()
                        ? ChooserProviderCaps.readableName(choice.provider)
                        : choice.source.displayName();

        auto* btn = new QRadioButton(name, this);
        m_providers.addButton(btn, i);
        ui->providersLayout->addWidget(btn);
    }
}

void ChooseProviderDialog::disableInput()
{
    for (auto& btn : m_providers.buttons())
    {
        btn->setEnabled(false);
    }

    ui->skipOneButton->setEnabled(false);
    ui->skipAllButton->setEnabled(false);
    ui->confirmOneButton->setEnabled(false);
    ui->confirmAllButton->setEnabled(false);
}

#include "moc_ChooseProviderDialog.cpp"
