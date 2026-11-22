// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "JavaInstallDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QStyle>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "Application.h"
#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/ProgressDialog.h"

using JavaRuntime::Provider;
using JavaRuntime::Release;

namespace
{

constexpr int ROLE_MAJOR = Qt::UserRole;
constexpr int ROLE_INDEX = Qt::UserRole + 1;

QTreeWidget* makeTree(QWidget* parent, const QStringList& headers)
{
    auto tree = new QTreeWidget(parent);
    tree->setHeaderLabels(headers);
    tree->setRootIsDecorated(false);
    tree->setUniformRowHeights(true);
    tree->setAlternatingRowColors(true);
    tree->setSelectionMode(QAbstractItemView::SingleSelection);
    tree->setAllColumnsShowFocus(true);
    return tree;
}

}  // namespace

JavaInstallDialog::JavaInstallDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Install Java"));
    resize(700, 500);

    m_providerBox = new QComboBox(this);
    for (auto provider : JavaRuntime::allProviders())
    {
        m_providerBox->addItem(JavaRuntime::providerName(provider),
                               JavaRuntime::providerId(provider));
    }
    auto providerLabel = new QLabel(tr("&Provider:"), this);
    providerLabel->setBuddy(m_providerBox);

    m_majorTree = makeTree(this, {tr("Major Version")});
    m_majorTree->setFixedWidth(130);

    m_releaseTree = makeTree(
        this, {tr("Version"), tr("Java Name"), tr("Released"), tr("Type")});
    m_releaseTree->header()->setStretchLastSection(false);
    m_releaseTree->header()->setSectionResizeMode(
        0, QHeaderView::ResizeToContents);
    m_releaseTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_releaseTree->header()->setSectionResizeMode(
        2, QHeaderView::ResizeToContents);
    m_releaseTree->header()->setSectionResizeMode(
        3, QHeaderView::ResizeToContents);

    m_majorSearch = new QLineEdit(this);
    m_majorSearch->setPlaceholderText(tr("Search"));
    m_majorSearch->setClearButtonEnabled(true);
    m_majorSearch->setFixedWidth(130);
    m_releaseSearch = new QLineEdit(this);
    m_releaseSearch->setPlaceholderText(tr("Search"));
    m_releaseSearch->setClearButtonEnabled(true);

    auto refreshButton = new QPushButton(tr("&Refresh"), this);
    m_recommended = new QCheckBox(tr("Re&commended"), this);
    m_recommended->setToolTip(
        tr("Only show long-term support versions and JRE builds, the ones "
           "Minecraft needs."));
    m_recommended->setChecked(true);
    m_downloadButton =
        new QPushButton(style()->standardIcon(QStyle::SP_DialogApplyButton),
                        tr("&Download"), this);
    m_downloadButton->setDefault(true);
    auto cancelButton =
        new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton),
                        tr("Cancel"), this);

    auto lists = new QGridLayout();
    lists->addWidget(m_majorTree, 0, 0);
    lists->addWidget(m_releaseTree, 0, 1);
    lists->addWidget(m_majorSearch, 1, 0);
    lists->addWidget(m_releaseSearch, 1, 1);
    lists->setColumnStretch(1, 1);
    lists->setRowStretch(0, 1);

    auto buttons = new QHBoxLayout();
    buttons->addWidget(refreshButton);
    buttons->addWidget(m_recommended);
    buttons->addStretch(1);
    buttons->addWidget(m_downloadButton);
    buttons->addWidget(cancelButton);

    auto providerRow = new QHBoxLayout();
    providerRow->addWidget(providerLabel);
    providerRow->addWidget(m_providerBox, 1);

    auto layout = new QVBoxLayout(this);
    layout->addLayout(providerRow);
    layout->addLayout(lists, 1);
    layout->addLayout(buttons);

    connect(m_providerBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &JavaInstallDialog::providerChanged);
    connect(m_majorTree, &QTreeWidget::currentItemChanged, this,
            &JavaInstallDialog::majorChanged);
    connect(m_releaseTree, &QTreeWidget::currentItemChanged, this,
            &JavaInstallDialog::updateButtons);
    connect(m_releaseTree, &QTreeWidget::itemDoubleClicked, this,
            &JavaInstallDialog::releaseDoubleClicked);
    connect(m_majorSearch, &QLineEdit::textChanged, this,
            &JavaInstallDialog::applyFilters);
    connect(m_releaseSearch, &QLineEdit::textChanged, this,
            &JavaInstallDialog::applyFilters);
    connect(m_recommended, &QCheckBox::toggled, this,
            &JavaInstallDialog::applyFilters);
    connect(refreshButton, &QPushButton::clicked, this,
            &JavaInstallDialog::refresh);
    connect(m_downloadButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    auto preferred =
        APPLICATION->settings()->get("AutomaticJavaProvider").toString();
    int index = m_providerBox->findData(preferred);
    if (index > 0)
    {
        m_providerBox->setCurrentIndex(index);
    }
    else
    {
        providerChanged();  // index 0 is already current, so no change signal
                            // fires
    }
    updateButtons();
}

Provider JavaInstallDialog::currentProvider() const
{
    return JavaRuntime::providerFromId(m_providerBox->currentData().toString())
        .value_or(Provider::Mojang);
}

std::optional<int> JavaInstallDialog::currentMajor() const
{
    auto item = m_majorTree->currentItem();
    if (!item || item->isHidden() || !item->data(0, ROLE_MAJOR).isValid())
    {
        return std::nullopt;
    }
    return item->data(0, ROLE_MAJOR).toInt();
}

std::optional<Release> JavaInstallDialog::selectedRelease() const
{
    auto item = m_releaseTree->currentItem();
    if (!item || item->isHidden() || !item->data(0, ROLE_INDEX).isValid())
    {
        return std::nullopt;
    }
    int index = item->data(0, ROLE_INDEX).toInt();
    if (index < 0 || index >= m_shownReleases.size())
    {
        return std::nullopt;
    }
    return m_shownReleases[index];
}

void JavaInstallDialog::setPlaceholder(QTreeWidget* tree, const QString& text)
{
    tree->clear();
    auto item = new QTreeWidgetItem(tree, {text});
    item->setFlags(Qt::NoItemFlags);
    item->setFirstColumnSpanned(true);
}

void JavaInstallDialog::providerChanged()
{
    loadMajors();
}

void JavaInstallDialog::majorChanged()
{
    loadReleases();
}

void JavaInstallDialog::refresh()
{
    auto provider = currentProvider();
    m_majors.remove(provider);
    for (auto it = m_releases.begin(); it != m_releases.end();)
    {
        if (it.key().first == provider)
        {
            it = m_releases.erase(it);
        }
        else
        {
            ++it;
        }
    }
    loadMajors();
}

void JavaInstallDialog::loadMajors()
{
    if (m_majorTask)
    {
        disconnect(m_majorTask.get(), nullptr, this, nullptr);
        m_majorTask->abort();
        m_majorTask.reset();
    }
    m_releaseTree->clear();
    m_shownReleases.clear();

    auto provider = currentProvider();
    if (m_majors.contains(provider))
    {
        showMajors();
        return;
    }

    setPlaceholder(m_majorTree, tr("Loading..."));
    m_majorTask.reset(new JavaRuntime::VersionListTask(provider));
    connect(m_majorTask.get(), &Task::finished, this,
            &JavaInstallDialog::majorsLoaded);
    m_majorTask->start();
}

void JavaInstallDialog::majorsLoaded()
{
    auto task = m_majorTask;
    m_majorTask.reset();
    if (!task->wasSuccessful())
    {
        setPlaceholder(m_majorTree, tr("Failed to load"));
        setPlaceholder(m_releaseTree, task->failReason());
        return;
    }
    m_majors.insert(task->provider(), task->majors());
    showMajors();
}

void JavaInstallDialog::showMajors()
{
    auto majors = m_majors.value(currentProvider());
    m_majorTree->clear();
    if (majors.isEmpty())
    {
        setPlaceholder(m_majorTree, tr("None available"));
        return;
    }
    for (int major : majors)
    {
        auto item =
            new QTreeWidgetItem(m_majorTree, {tr("Java %1").arg(major)});
        item->setData(0, ROLE_MAJOR, major);
    }
    applyFilters();
}

void JavaInstallDialog::loadReleases()
{
    if (m_releaseTask)
    {
        disconnect(m_releaseTask.get(), nullptr, this, nullptr);
        m_releaseTask->abort();
        m_releaseTask.reset();
    }
    m_shownReleases.clear();
    m_releaseTree->clear();
    updateButtons();

    auto major = currentMajor();
    if (!major)
    {
        return;
    }

    auto key = qMakePair(currentProvider(), *major);
    if (m_releases.contains(key))
    {
        showReleases();
        return;
    }

    setPlaceholder(m_releaseTree, tr("Loading..."));
    m_releaseTask.reset(
        new JavaRuntime::ReleaseListTask(key.first, key.second));
    connect(m_releaseTask.get(), &Task::finished, this,
            &JavaInstallDialog::releasesLoaded);
    m_releaseTask->start();
}

void JavaInstallDialog::releasesLoaded()
{
    auto task = m_releaseTask;
    m_releaseTask.reset();
    if (!task->wasSuccessful())
    {
        setPlaceholder(m_releaseTree,
                       tr("Failed to load: %1").arg(task->failReason()));
        return;
    }
    m_releases.insert(qMakePair(task->provider(), task->major()),
                      task->releases());
    showReleases();
}

void JavaInstallDialog::releaseDoubleClicked()
{
    if (selectedRelease())
    {
        accept();
    }
}

void JavaInstallDialog::showReleases()
{
    auto major = currentMajor();
    if (!major)
    {
        return;
    }

    m_shownReleases = m_releases.value(qMakePair(currentProvider(), *major));
    m_releaseTree->clear();
    if (m_shownReleases.isEmpty())
    {
        setPlaceholder(m_releaseTree, tr("No builds for this platform"));
        updateButtons();
        return;
    }

    QLocale locale;
    for (int i = 0; i < m_shownReleases.size(); i++)
    {
        auto& release = m_shownReleases[i];
        auto released =
            release.released.isValid()
                ? locale.toString(release.released.date(), QLocale::ShortFormat)
                : QString();
        auto item = new QTreeWidgetItem(
            m_releaseTree,
            {release.version, release.name, released, release.type});
        item->setData(0, ROLE_INDEX, i);
        item->setToolTip(0, release.version);
        item->setToolTip(1, release.url);
    }
    applyFilters();
}

void JavaInstallDialog::applyFilters()
{
    bool recommended = m_recommended->isChecked();

    auto majorFilter = m_majorSearch->text().trimmed();
    QTreeWidgetItem* firstVisibleMajor = nullptr;
    for (int i = 0; i < m_majorTree->topLevelItemCount(); i++)
    {
        auto item = m_majorTree->topLevelItem(i);
        if (!item->data(0, ROLE_MAJOR).isValid())
        {
            continue;
        }
        int major = item->data(0, ROLE_MAJOR).toInt();
        bool visible =
            (!recommended || JavaRuntime::isLts(major)) &&
            (majorFilter.isEmpty() ||
             item->text(0).contains(majorFilter, Qt::CaseInsensitive));
        item->setHidden(!visible);
        if (visible && !firstVisibleMajor)
        {
            firstVisibleMajor = item;
        }
    }
    auto currentMajorItem = m_majorTree->currentItem();
    if ((!currentMajorItem || currentMajorItem->isHidden()) &&
        firstVisibleMajor)
    {
        m_majorTree->setCurrentItem(firstVisibleMajor);
    }

    auto releaseFilter = m_releaseSearch->text().trimmed();
    QTreeWidgetItem* firstVisibleRelease = nullptr;
    for (int i = 0; i < m_releaseTree->topLevelItemCount(); i++)
    {
        auto item = m_releaseTree->topLevelItem(i);
        if (!item->data(0, ROLE_INDEX).isValid())
        {
            continue;
        }
        auto& release = m_shownReleases[item->data(0, ROLE_INDEX).toInt()];
        bool matches =
            releaseFilter.isEmpty() ||
            release.version.contains(releaseFilter, Qt::CaseInsensitive) ||
            release.name.contains(releaseFilter, Qt::CaseInsensitive);
        bool visible = (!recommended || release.type == "jre") && matches;
        item->setHidden(!visible);
        if (visible && !firstVisibleRelease)
        {
            firstVisibleRelease = item;
        }
    }
    auto currentReleaseItem = m_releaseTree->currentItem();
    if ((!currentReleaseItem || currentReleaseItem->isHidden()) &&
        firstVisibleRelease)
    {
        m_releaseTree->setCurrentItem(firstVisibleRelease);
    }

    updateButtons();
}

void JavaInstallDialog::updateButtons()
{
    m_downloadButton->setEnabled(selectedRelease().has_value());
}

std::optional<JavaRuntime::InstalledRuntime> JavaInstallDialog::pickAndInstall(
    QWidget* parent)
{
    JavaInstallDialog dialog(parent);
    if (dialog.exec() != QDialog::Accepted)
    {
        return std::nullopt;
    }
    auto release = dialog.selectedRelease();
    if (!release)
    {
        return std::nullopt;
    }

    auto task = std::make_unique<JavaRuntime::InstallTask>(*release);
    ProgressDialog progress(parent);
    progress.execWithTask(task.get());
    if (task->wasSuccessful())
    {
        return task->result();
    }
    if (task->getState() != Task::State::AbortedByUser)
    {
        CustomMessageBox::selectable(parent, tr("Java installation failed"),
                                     task->failReason(), QMessageBox::Critical)
            ->exec();
    }
    return std::nullopt;
}
