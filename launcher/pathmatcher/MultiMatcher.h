#include "IPathMatcher.h"
#include <QRegularExpression>
#include <SeparatorPrefixTree.h>

class MultiMatcher : public IPathMatcher
{
public:
    ~MultiMatcher() override = default;
    MultiMatcher() = default;
    MultiMatcher &add(Ptr add)
    {
        m_matchers.append(add);
        return *this;
    }

    bool matches(const QString &string) const override
    {
        for(auto iter: m_matchers)
        {
            if(iter->matches(string))
            {
                return true;
            }
        }
        return false;
    }

    QList<Ptr> m_matchers;
};
