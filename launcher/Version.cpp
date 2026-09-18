#include "Version.h"

#include <QStringList>
#include <QUrl>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

Version::Version(const QString &str) : m_string(str)
{
    parse();
}

bool Version::operator<(const Version &other) const
{
    if (m_era != other.m_era) {
        return m_era < other.m_era;
    }

    const int size = qMax(m_sections.size(), other.m_sections.size());
    for (int i = 0; i < size; ++i)
    {
        const Section sec1 = (i >= m_sections.size()) ? Section("0") : m_sections.at(i);
        const Section sec2 =
            (i >= other.m_sections.size()) ? Section("0") : other.m_sections.at(i);
        if (sec1 != sec2)
        {
            return sec1 < sec2;
        }
    }

    return false;
}
bool Version::operator<=(const Version &other) const
{
    return *this < other || *this == other;
}
bool Version::operator>(const Version &other) const
{
    if (m_era != other.m_era) {
        return m_era > other.m_era;
    }

    const int size = qMax(m_sections.size(), other.m_sections.size());
    for (int i = 0; i < size; ++i)
    {
        const Section sec1 = (i >= m_sections.size()) ? Section("0") : m_sections.at(i);
        const Section sec2 =
            (i >= other.m_sections.size()) ? Section("0") : other.m_sections.at(i);
        if (sec1 != sec2)
        {
            return sec1 > sec2;
        }
    }

    return false;
}
bool Version::operator>=(const Version &other) const
{
    return *this > other || *this == other;
}
bool Version::operator==(const Version &other) const
{
    if (m_era != other.m_era) {
        return false;
    }

    const int size = qMax(m_sections.size(), other.m_sections.size());
    for (int i = 0; i < size; ++i)
    {
        const Section sec1 = (i >= m_sections.size()) ? Section("0") : m_sections.at(i);
        const Section sec2 =
            (i >= other.m_sections.size()) ? Section("0") : other.m_sections.at(i);
        if (sec1 != sec2)
        {
            return false;
        }
    }

    return true;
}
bool Version::operator!=(const Version &other) const
{
    return !operator==(other);
}

bool Version::isPreAuthlib() const
{
    if (m_era < 0) {
        return true;
    }

    static const QRegularExpression snapshotRegex(R"(^(\d{2})w(\d{2})[a-z]?$)");
    const auto match = snapshotRegex.match(m_string.trimmed());
    if (match.hasMatch()) {
        const int year = match.captured(1).toInt();
        const int week = match.captured(2).toInt();
        if (year < 13 || (year == 13 && week < 41)) {
            return true;
        }
        return false;
    }

    return *this < Version("1.7.2");
}

void Version::parse()
{
    m_sections.clear();

    QString cleanStr = m_string.trimmed();
    if (cleanStr.startsWith("Minecraft ", Qt::CaseInsensitive)) {
        cleanStr = cleanStr.mid(10).trimmed();
    }

    if (cleanStr.startsWith("rd-", Qt::CaseInsensitive)) {
        m_era = -5;
        cleanStr = cleanStr.mid(3);
    } else if (cleanStr.startsWith("c", Qt::CaseInsensitive) && cleanStr.length() > 1 && cleanStr[1].isDigit()) {
        m_era = -4;
        cleanStr = cleanStr.mid(1);
    } else if (cleanStr.startsWith("in-", Qt::CaseInsensitive)) {
        m_era = -3;
        cleanStr = cleanStr.mid(3);
    } else if (cleanStr.startsWith("a", Qt::CaseInsensitive) && cleanStr.length() > 1 && cleanStr[1].isDigit()) {
        m_era = -2;
        cleanStr = cleanStr.mid(1);
    } else if (cleanStr.startsWith("b", Qt::CaseInsensitive) && cleanStr.length() > 1 && cleanStr[1].isDigit()) {
        m_era = -1;
        cleanStr = cleanStr.mid(1);
    } else {
        m_era = 0;
    }

    QStringList parts = cleanStr.split('.');

    for (const auto& part : parts)
    {
        m_sections.append(Section(part));
    }
}
