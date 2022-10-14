#include "ManagedPackPage.h"
#include "ui_ManagedPackPage.h"

#include <QListView>
#include <QProxyStyle>

#include "Application.h"
#include "BuildConfig.h"
#include "InstanceImportTask.h"
#include "InstanceList.h"
#include "InstanceTask.h"
#include "Json.h"

#include "modplatform/modrinth/ModrinthPackManifest.h"

#include "ui/InstanceWindow.h"
#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/ProgressDialog.h"

/** This is just to override the combo box popup behavior so that the combo box doesn't take the whole screen.
 *  ... thanks Qt.
 */
class NoBigComboBoxStyle : public QProxyStyle {
    Q_OBJECT

   public:
    NoBigComboBoxStyle(QStyle* style) : QProxyStyle(style) {}

    // clang-format off
    int styleHint(QStyle::StyleHint hint, const QStyleOption* option = nullptr, const QWidget* widget = nullptr, QStyleHintReturn* returnData = nullptr) const override
    {
        if (hint == QStyle::SH_ComboBox_Popup)
            return false;

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
    // clang-format on
};

ManagedPackPage* ManagedPackPage::createPage(BaseInstance* inst, QString type, QWidget* parent)
{
    if (type == "modrinth")
        return new ModrinthManagedPackPage(inst, nullptr, parent);
    if (type == "flame")
        return new FlameManagedPackPage(inst, nullptr, parent);

    return new GenericManagedPackPage(inst, nullptr, parent);
}

ManagedPackPage::ManagedPackPage(BaseInstance* inst, InstanceWindow* instance_window, QWidget* parent)
    : QWidget(parent), m_instance_window(instance_window), ui(new Ui::ManagedPackPage), m_inst(inst)
{
    Q_ASSERT(inst);

    ui->setupUi(this);

    ui->versionsComboBox->setStyle(new NoBigComboBoxStyle(ui->versionsComboBox->style()));
}

ManagedPackPage::~ManagedPackPage()
{
    delete ui;
}

void ManagedPackPage::openedImpl()
{
    ui->packName->setText(m_inst->getManagedPackName());
    ui->packVersion->setText(m_inst->getManagedPackVersionName());
    ui->packOrigin->setText(tr("Website: %1    |    Pack ID: %2    |    Version ID: %3")
                                .arg(displayName(), m_inst->getManagedPackID(), m_inst->getManagedPackVersionID()));

    parseManagedPack();
}

QString ManagedPackPage::displayName() const
{
    auto type = m_inst->getManagedPackType();
    if (type.isEmpty())
        return {};
    return type.replace(0, 1, type[0].toUpper());
}

QIcon ManagedPackPage::icon() const
{
    return APPLICATION->getThemedIcon(m_inst->getManagedPackType());
}

QString ManagedPackPage::helpPage() const
{
    return {};
}

void ManagedPackPage::retranslate()
{
    ui->retranslateUi(this);
}

bool ManagedPackPage::shouldDisplay() const
{
    return m_inst->isManagedPack();
}

bool ManagedPackPage::runUpdateTask(InstanceTask* task)
{
    Q_ASSERT(task);

    unique_qobject_ptr<Task> wrapped_task(APPLICATION->instances()->wrapInstanceTask(task));

    connect(task, &Task::failed,
            [this](QString reason) { CustomMessageBox::selectable(this, tr("Error"), reason, QMessageBox::Critical)->show(); });
    connect(task, &Task::succeeded, [this, task]() {
        QStringList warnings = task->warnings();
        if (warnings.count())
            CustomMessageBox::selectable(this, tr("Warnings"), warnings.join('\n'), QMessageBox::Warning)->show();
    });
    connect(task, &Task::aborted, [this] {
        CustomMessageBox::selectable(this, tr("Task aborted"), tr("The task has been aborted by the user."), QMessageBox::Information)
            ->show();
    });

    ProgressDialog loadDialog(this);
    loadDialog.setSkipButton(true, tr("Abort"));
    loadDialog.execWithTask(task);

    return task->wasSuccessful();
}

ModrinthManagedPackPage::ModrinthManagedPackPage(BaseInstance* inst, InstanceWindow* instance_window, QWidget* parent)
    : ManagedPackPage(inst, instance_window, parent)
{
    Q_ASSERT(inst->isManagedPack());
    connect(ui->versionsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(suggestVersion()));
    connect(ui->updateButton, &QPushButton::pressed, this, &ModrinthManagedPackPage::update);
}

void ModrinthManagedPackPage::parseManagedPack()
{
    qDebug() << "Parsing Modrinth pack";

    auto netJob = new NetJob(QString("Modrinth::PackVersions(%1)").arg(m_inst->getManagedPackName()), APPLICATION->network());
    auto response = new QByteArray();

    QString id = m_inst->getManagedPackID();

    netJob->addNetAction(Net::Download::makeByteArray(QString("%1/project/%2/version").arg(BuildConfig.MODRINTH_PROD_URL, id), response));

    QObject::connect(netJob, &NetJob::succeeded, this, [this, response, id] {
        QJsonParseError parse_error{};
        QJsonDocument doc = QJsonDocument::fromJson(*response, &parse_error);
        if (parse_error.error != QJsonParseError::NoError) {
            qWarning() << "Error while parsing JSON response from Modrinth at " << parse_error.offset
                       << " reason: " << parse_error.errorString();
            qWarning() << *response;
            return;
        }

        try {
            Modrinth::loadIndexedVersions(m_pack, doc);
        } catch (const JSONValidationError& e) {
            qDebug() << *response;
            qWarning() << "Error while reading modrinth modpack version: " << e.cause();
        }

        for (auto version : m_pack.versions) {
            QString name;

            if (!version.name.contains(version.version))
                name = QString("%1 — %2").arg(version.name, version.version);
            else
                name = version.name;

            // NOTE: the id from version isn't the same id in the modpack format spec...
            // e.g. HexMC's 4.4.0 has versionId 4.0.0 in the modpack index..............
            if (version.version == m_inst->getManagedPackVersionName())
                name.append(tr(" (Current)"));

            ui->versionsComboBox->addItem(name, QVariant(version.id));
        }

        suggestVersion();

        m_loaded = true;
    });
    QObject::connect(netJob, &NetJob::finished, this, [response, netJob] {
        netJob->deleteLater();
        delete response;
    });
    netJob->start();
}

QString ModrinthManagedPackPage::url() const
{
    return {};
}

void ModrinthManagedPackPage::suggestVersion()
{
    auto index = ui->versionsComboBox->currentIndex();
    auto version = m_pack.versions.at(index);

    ui->changelogTextBrowser->setText(version.changelog);
}

void ModrinthManagedPackPage::update()
{
    auto index = ui->versionsComboBox->currentIndex();
    auto version = m_pack.versions.at(index);

    auto extracted = new InstanceImportTask(version.download_url, this);

    InstanceName inst_name(m_inst->getManagedPackName(), version.version);
    inst_name.setName(m_inst->name().replace(m_inst->getManagedPackVersionName(), version.version));
    extracted->setName(inst_name);

    extracted->setGroup(APPLICATION->instances()->getInstanceGroup(m_inst->id()));
    extracted->setIcon(m_inst->iconKey());
    extracted->setConfirmUpdate(false);

    auto did_succeed = runUpdateTask(extracted);

    if (m_instance_window && did_succeed)
        m_instance_window->close();
}

FlameManagedPackPage::FlameManagedPackPage(BaseInstance* inst, InstanceWindow* instance_window, QWidget* parent)
    : ManagedPackPage(inst, instance_window, parent)
{
    Q_ASSERT(inst->isManagedPack());
    connect(ui->versionsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(suggestVersion()));
}

void FlameManagedPackPage::parseManagedPack() {}

QString FlameManagedPackPage::url() const
{
    return {};
}

void FlameManagedPackPage::suggestVersion()
{
}

#include "ManagedPackPage.moc"
