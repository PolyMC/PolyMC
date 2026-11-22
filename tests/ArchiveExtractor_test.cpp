#include <QTest>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <GZip.h>
#include <java/download/ArchiveExtractor.h>

namespace
{

// Builds a tar archive in memory, one entry at a time.
void writeOctal(QByteArray& header, int offset, int size, qint64 value)
{
    auto text = QByteArray::number(value, 8).rightJustified(size - 1, '0');
    memcpy(header.data() + offset, text.constData(), text.size());
}

class TarBuilder
{
public:
    void add(const QByteArray& name, char type, const QByteArray& content = {},
             const QByteArray& link = {}, int mode = 0644)
    {
        QByteArray header(512, '\0');
        memcpy(header.data(), name.constData(), qMin<int>(name.size(), 100));
        writeOctal(header, 100, 8, mode);
        writeOctal(header, 108, 8, 0);
        writeOctal(header, 116, 8, 0);
        writeOctal(header, 124, 12, content.size());
        writeOctal(header, 136, 12, 0);
        header[156] = type;
        memcpy(header.data() + 157, link.constData(),
               qMin<int>(link.size(), 100));
        memcpy(header.data() + 257,
               "ustar\0"
               "00",
               8);
        memset(header.data() + 148, ' ', 8);
        int sum = 0;
        for (char c : header)
        {
            sum += static_cast<unsigned char>(c);
        }
        auto chksum = QByteArray::number(sum, 8).rightJustified(6, '0');
        memcpy(header.data() + 148, chksum.constData(), 6);
        header[154] = '\0';

        m_data += header;
        m_data += content;
        m_data += QByteArray((512 - content.size() % 512) % 512, '\0');
    }

    void addLongName(const QByteArray& name, const QByteArray& content)
    {
        add("././@LongLink", 'L', name + '\0');
        add(name.left(100), '0', content);
    }

    void addPax(const QByteArray& path, const QByteArray& content)
    {
        QByteArray record = " path=" + path + "\n";
        // the length prefix counts its own digits
        int length = record.size();
        while (QByteArray::number(length).size() + record.size() != length)
        {
            length = QByteArray::number(length).size() + record.size();
        }
        add("PaxHeader", 'x', QByteArray::number(length) + record);
        add("ignored-short-name", '0', content);
    }

    QByteArray gzipped() const
    {
        QByteArray out;
        GZip::zip(m_data + QByteArray(1024, '\0'), out);
        return out;
    }

private:
    QByteArray m_data;
};

QString writeArchive(const QTemporaryDir& dir, const QByteArray& data)
{
    auto path = dir.filePath("archive.tar.gz");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        return {};
    }
    file.write(data);
    return path;
}

}  // namespace

class ArchiveExtractorTest : public QObject
{
    Q_OBJECT

private slots:
    void test_extractTarGz()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        QByteArray longName = "jdk/" + QByteArray(120, 'a') + "/file.txt";
        TarBuilder tar;
        tar.add("jdk/", '5', {}, {}, 0755);
        tar.add("jdk/bin/java", '0', "#!/bin/sh\n", {}, 0755);
        tar.add("jdk/release", '0', "JAVA_VERSION=\"17\"\n");
        tar.add("jdk/bin/link", '2', {}, "java");
        tar.addLongName(longName, "long");
        tar.addPax("jdk/pax/named.txt", "pax");

        auto target = dir.filePath("out");
        QCOMPARE(
            ArchiveExtractor::extract(writeArchive(dir, tar.gzipped()), target),
            QString());

        QDir out(target);
        QVERIFY(QFileInfo(out.filePath("jdk/bin/java")).isFile());
#ifndef Q_OS_WIN
        // Windows decides executability by file extension, there is no mode bit
        // to check
        QVERIFY(QFileInfo(out.filePath("jdk/bin/java")).isExecutable());
#endif
        QFile release(out.filePath("jdk/release"));
        QVERIFY(release.open(QIODevice::ReadOnly));
        QCOMPARE(release.readAll(), QByteArray("JAVA_VERSION=\"17\"\n"));
        QVERIFY(QFileInfo::exists(out.filePath(QString::fromUtf8(longName))));
        QVERIFY(QFileInfo::exists(out.filePath("jdk/pax/named.txt")));
        QVERIFY(!QFileInfo::exists(out.filePath("ignored-short-name")));
#ifndef Q_OS_WIN
        QVERIFY(QFileInfo(out.filePath("jdk/bin/link")).isSymLink());
#endif
    }

    void test_rejectsTraversal_data()
    {
        QTest::addColumn<QByteArray>("name");
        QTest::addColumn<char>("type");
        QTest::addColumn<QByteArray>("link");

        QTest::newRow("dotdot") << QByteArray("../evil") << '0' << QByteArray();
        QTest::newRow("nested dotdot")
            << QByteArray("jdk/../../evil") << '0' << QByteArray();
        QTest::newRow("absolute")
            << QByteArray("/tmp/evil") << '0' << QByteArray();
        QTest::newRow("symlink out")
            << QByteArray("jdk/link") << '2' << QByteArray("../../evil");
        QTest::newRow("absolute symlink")
            << QByteArray("jdk/link") << '2' << QByteArray("/etc/passwd");
        QTest::newRow("hardlink out")
            << QByteArray("jdk/link") << '1' << QByteArray("../evil");
#ifdef Q_OS_WIN
        QTest::newRow("drive relative")
            << QByteArray("C:evil") << '0' << QByteArray();
        QTest::newRow("data stream")
            << QByteArray("jdk/file:stream") << '0' << QByteArray();
#endif
    }

    void test_rejectsTraversal()
    {
        QFETCH(QByteArray, name);
        QFETCH(char, type);
        QFETCH(QByteArray, link);

        QTemporaryDir dir;
        TarBuilder tar;
        tar.add(name, type, type == '0' ? QByteArray("bad") : QByteArray(),
                link);

        auto target = dir.filePath("out");
        QVERIFY(
            !ArchiveExtractor::extract(writeArchive(dir, tar.gzipped()), target)
                 .isEmpty());
        QVERIFY(!QFileInfo::exists(dir.filePath("evil")));
    }

    void test_truncated()
    {
        QTemporaryDir dir;
        TarBuilder tar;
        tar.add("jdk/bin/java", '0', QByteArray(4096, 'x'));
        auto data = tar.gzipped();
        data.chop(data.size() / 2);
        QVERIFY(!ArchiveExtractor::extract(writeArchive(dir, data),
                                           dir.filePath("out"))
                     .isEmpty());
    }

    void test_unknownFormat()
    {
        QTemporaryDir dir;
        QVERIFY(!ArchiveExtractor::extract(writeArchive(dir, "not an archive"),
                                           dir.filePath("out"))
                     .isEmpty());
    }
};

QTEST_GUILESS_MAIN(ArchiveExtractorTest)

#include "ArchiveExtractor_test.moc"
