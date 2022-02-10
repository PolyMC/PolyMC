#pragma once
#include <QString>
#include <memory>

class IPathMatcher
{
public:
    using Ptr = std::shared_ptr<IPathMatcher>;

public:
    virtual ~IPathMatcher() = default;
    virtual bool matches(const QString &string) const = 0;
};
