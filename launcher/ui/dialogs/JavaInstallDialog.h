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

#pragma once

#include <QDialog>
#include <QMap>

#include "java/download/JavaRuntimeTasks.h"

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;
class QTreeWidget;

/**
 * Lets the user browse the Java builds of every provider and pick one to
 * install. Provider dropdown on top, major versions and their builds below it.
 */
class JavaInstallDialog : public QDialog
{
    Q_OBJECT
public:
    explicit JavaInstallDialog(QWidget* parent = nullptr);

    std::optional<JavaRuntime::Release> selectedRelease() const;

    /**
     * Shows the dialog and installs the picked build with a progress dialog.
     * Returns the new runtime, or nothing if the user cancelled or the install
     * failed (errors are shown).
     */
    static std::optional<JavaRuntime::InstalledRuntime> pickAndInstall(
        QWidget* parent);

private slots:
    void majorsLoaded();
    void releasesLoaded();
    void releaseDoubleClicked();

private:
    JavaRuntime::Provider currentProvider() const;
    std::optional<int> currentMajor() const;

    void createWidgets();
    void createLayout();
    void connectSignals();
    void selectPreferredProvider();

    void providerChanged();
    void majorChanged();
    void refresh();

    void loadMajors();
    void loadReleases();
    void showMajors();
    void showReleases();
    void applyFilters();
    void updateButtons();

    void setPlaceholder(QTreeWidget* tree, const QString& text);

    QComboBox* m_providerBox;
    QTreeWidget* m_majorTree;
    QTreeWidget* m_releaseTree;
    QLineEdit* m_majorSearch;
    QLineEdit* m_releaseSearch;
    QCheckBox* m_recommended;
    QPushButton* m_downloadButton;
    QPushButton* m_refreshButton;
    QPushButton* m_cancelButton;

    shared_qobject_ptr<JavaRuntime::VersionListTask> m_majorTask;
    shared_qobject_ptr<JavaRuntime::ReleaseListTask> m_releaseTask;

    // results are kept while the dialog is open, Refresh drops them for the
    // current provider
    QMap<JavaRuntime::Provider, QList<int>> m_majors;
    QMap<QPair<JavaRuntime::Provider, int>, QList<JavaRuntime::Release>>
        m_releases;
    QList<JavaRuntime::Release> m_shownReleases;
};
