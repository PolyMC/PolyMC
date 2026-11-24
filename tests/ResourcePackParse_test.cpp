// SPDX-License-Identifier: GPL-3.0-only
/*
 *  PolyMC - Minecraft Launcher
 *  Copyright (c) 2022 flowln <flowlnlnln@gmail.com>
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

#include <QTest>
#include <QFileInfo>

#include <FileSystem.h>

#include <minecraft/mod/DataPack.h>
#include <minecraft/mod/ResourcePack.h>
#include <minecraft/mod/tasks/LocalResourcePackParseTask.h>

class ResourcePackParseTest : public QObject
{
    Q_OBJECT

private slots:
    void test_parseZIP()
    {
        QString source = QFINDTESTDATA("testdata/ResourcePackParse");

        QString zip_rp = FS::PathCombine(source, "test_resource_pack_idk.zip");
        ResourcePack pack{QFileInfo(zip_rp)};

        ResourcePackUtils::processZIP(pack);

        QVERIFY(pack.packFormat() == 3);
        QVERIFY(pack.description() ==
                "um dois, feijão com arroz, três quatro, feijão no prato, "
                "cinco seis, café inglês, sete oito, comer biscoito, nove dez "
                "comer pastéis!!");
    }

    void test_parseFolder()
    {
        QString source = QFINDTESTDATA("testdata/ResourcePackParse");

        QString folder_rp = FS::PathCombine(source, "test_folder");
        ResourcePack pack{QFileInfo(folder_rp)};

        ResourcePackUtils::processFolder(pack);

        QVERIFY(pack.packFormat() == 1);
        QVERIFY(pack.description() == "Some resource pack maybe");
    }

    void test_parseFolder2()
    {
        QString source = QFINDTESTDATA("testdata/ResourcePackParse");

        QString folder_rp = FS::PathCombine(source, "another_test_folder");
        ResourcePack pack{QFileInfo(folder_rp)};

        ResourcePackUtils::process(pack);

        QVERIFY(pack.packFormat() == 6);
        QVERIFY(pack.description() ==
                "o quartel pegou fogo, policia deu sinal, acode acode acode a "
                "bandeira nacional");
    }

    void test_compatibleVersions()
    {
        ResourcePack pack;

        pack.setPackFormat(3, {0, 0}, {0, 0});
        QCOMPARE(pack.toString(), QString("3 (1.11 - 1.12.2)"));

        pack.setPackFormat(100, {0, 0}, {0, 0});
        QCOMPARE(pack.toString(), QString("100 (26.4-snap3 - 26.4-snap3)"));

        pack.setPackFormat(9999, {0, 0}, {0, 0});
        QCOMPARE(pack.toString(), QString("9999"));

        pack.setPackFormat(0, {4, 0}, {6, 0});
        QCOMPARE(pack.toString(), QString("4 - 6 (1.13 - 1.16.5)"));

        pack.setPackFormat(0, {97, 1}, {97, 1});
        QCOMPARE(pack.toString(), QString("97.1 (26.3)"));

        pack.setPackFormat(0, {4, 0}, {9999, 0});
        QCOMPARE(pack.toString(), QString("4 - 9999 (1.13 - 26.4-snap3)"));

        DataPack data;

        data.setPackFormat(3, {0, 0}, {0, 0});
        QCOMPARE(data.toString(), QString("3"));

        data.setPackFormat(4, {0, 0}, {0, 0});
        QCOMPARE(data.toString(), QString("4 (1.13 - 1.14.4)"));

        data.setPackFormat(0, {94, 1}, {101, 1});
        QCOMPARE(data.toString(),
                 QString("94.1 - 101.1 (1.21.11-pre4 - 26.1.2)"));
    }
};

QTEST_GUILESS_MAIN(ResourcePackParseTest)

#include "ResourcePackParse_test.moc"
