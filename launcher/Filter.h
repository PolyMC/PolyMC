#pragma once

#include <QString>
#include <QRegularExpression>

class Filter
{
public:
    virtual ~Filter();
    virtual bool accepts(const QString & value) = 0;
};

class ContainsFilter: public Filter
{
public:
    ContainsFilter(const QString &pattern);
    ~ContainsFilter() override;
    bool accepts(const QString & value) override;
private:
    QString pattern;
};

class ExactFilter: public Filter
{
public:
    ExactFilter(const QString &pattern);
    ~ExactFilter() override;
    bool accepts(const QString & value) override;
private:
    QString pattern;
};

class RegexpFilter: public Filter
{
public:
    RegexpFilter(const QString &regexp, bool invert);
    ~RegexpFilter() override;
    bool accepts(const QString & value) override;
private:
    QRegularExpression pattern;
    bool invert = false;
};
