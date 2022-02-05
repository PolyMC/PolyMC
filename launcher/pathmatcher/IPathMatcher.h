#pragma once
#include <QString>
#include <memory>

class IPathMatcher
{
public:
    typedef std::shared_ptr<IPathMatcher> Ptr;

public:
    virtual ~IPathMatcher() = default;
    virtual bool matches(const QString &string) const = 0;
};
