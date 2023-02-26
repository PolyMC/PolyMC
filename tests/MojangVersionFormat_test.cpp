#include <QTest>
#include <QDebug>

#include <minecraft/MojangVersionFormat.h>

class MojangVersionFormatTest : public QObject
{
    Q_OBJECT

    static nlohmann::json readJson(const QString path)
    {
        /*
        QFile jsonFile(path);
        jsonFile.open(QIODevice::ReadOnly);
        auto data = jsonFile.readAll();
        jsonFile.close();
        return QJsonDocument::fromJson(data);
        */
        std::ifstream jsonFile(path.toStdString());
        nlohmann::json doc = nlohmann::json::parse(jsonFile);
        jsonFile.close();
        return doc;
    }
    static void writeJson(const char *file, nlohmann::json doc)
    {
        /*
        QFile jsonFile(file);
        jsonFile.open(QIODevice::WriteOnly | QIODevice::Text);
        auto data = doc.toJson(QJsonDocument::Indented);
        qDebug() << QString::fromUtf8(data);
        jsonFile.write(data);
        jsonFile.close();
        */
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

        //QCOMPARE(doc.toJson(), doc2.toJson());
        QCOMPARE(doc.dump(), doc2.dump());
    }

    void test_Through()
    {
        nlohmann::json doc = readJson(QFINDTESTDATA("testdata/MojangVersionFormat/1.9.json"));
        auto vfile = MojangVersionFormat::versionFileFromJson(doc, "1.9.json");
        auto doc2 = MojangVersionFormat::versionFileToJson(vfile);
        writeJson("1.9-passthorugh.json", doc2);
        //QCOMPARE(doc.toJson(), doc2.toJson());
        QCOMPARE(doc.dump(), doc2.dump());
    }
};

QTEST_GUILESS_MAIN(MojangVersionFormatTest)

#include "MojangVersionFormat_test.moc"

