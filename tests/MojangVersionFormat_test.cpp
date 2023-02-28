#include <QTest>
#include <QDebug>

#include <minecraft/MojangVersionFormat.h>

class MojangVersionFormatTest : public QObject
{
    Q_OBJECT

    static nlohmann::json readJson(const QString path)
    {
        std::ifstream jsonFile(path.toStdString());
        nlohmann::json doc = nlohmann::json::parse(jsonFile);
        jsonFile.close();
        return doc;
    }
    static void writeJson(const char *file, nlohmann::json doc)
    {
        std::ofstream jsonFile(file);
        jsonFile << doc.dump(4);
        jsonFile.close();
    }

private
slots:
    void test_Through_Simple()
    {
        nlohmann::json doc = readJson(QFINDTESTDATA("testdata/MojangVersionFormat/1.9-simple.json"));
        auto vfile = MojangVersionFormat::versionFileFromJson(doc, "1.9-simple.json");
        auto doc2 = MojangVersionFormat::versionFileToJson(vfile);
        writeJson("1.9-simple-passthorugh.json", doc2);

        QCOMPARE(doc.dump(), doc2.dump());
    }

    void test_Through()
    {
        nlohmann::json doc = readJson(QFINDTESTDATA("testdata/MojangVersionFormat/1.9.json"));
        auto vfile = MojangVersionFormat::versionFileFromJson(doc, "1.9.json");
        auto doc2 = MojangVersionFormat::versionFileToJson(vfile);
        writeJson("1.9-passthorugh.json", doc2);

        QCOMPARE(doc.dump(), doc2.dump());
    }
};

QTEST_GUILESS_MAIN(MojangVersionFormatTest)

#include "MojangVersionFormat_test.moc"

