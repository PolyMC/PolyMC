#include "Filter.h"

#include <utility>

Filter::~Filter() = default;

ContainsFilter::ContainsFilter(QString pattern) : pattern(std::move(pattern)){}
ContainsFilter::~ContainsFilter() = default;
bool ContainsFilter::accepts(const QString& value)
{
    return value.contains(pattern);
}

ExactFilter::ExactFilter(QString pattern) : pattern(std::move(pattern)){}
ExactFilter::~ExactFilter() = default;
bool ExactFilter::accepts(const QString& value)
{
    return value == pattern;
}

RegexpFilter::RegexpFilter(const QString& regexp, bool invert)
    :invert(invert)
{
    pattern.setPattern(regexp);
    pattern.optimize();
}
RegexpFilter::~RegexpFilter() = default;
bool RegexpFilter::accepts(const QString& value)
{
    auto match = pattern.match(value);
    bool matched = match.hasMatch();
    return invert ? (!matched) : (matched);
}
