// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2026 crueter <crueter@crueter.xyz>
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
 *
 */

#include "WorldDataPacksDialog.h"
#include "ui/pages/instance/DataPackPage.h"

WorldDataPacksDialog::WorldDataPacksDialog(MinecraftInstance* inst, const QString& worldName,
                                           const QString& datapacksDir, QWidget* parent)
    : QDialog(parent), m_model(std::make_shared<DataPackFolderModel>(datapacksDir)),
      m_verticalLayout(new QVBoxLayout(this)), m_page(new DataPackPage(inst, m_model, this)) {
    setObjectName(QStringLiteral("WorldDataPacksDialog"));
    m_verticalLayout->setObjectName(QStringLiteral("verticalLayout"));

    m_page->setWindowFlag(Qt::Window, false);
    m_verticalLayout->addWidget(m_page);
    m_page->show();

    resize(std::max(0.75 * parent->width(), 400.0), std::max(0.75 * parent->height(), 400.0));

    setWindowTitle(tr("Datapacks - %1").arg(worldName));
    setWindowModality(Qt::WindowModal);

    m_page->setFilter("%1 (*.zip)");
    m_page->openedImpl();
}

WorldDataPacksDialog::~WorldDataPacksDialog() {
    m_page->closedImpl();
}
