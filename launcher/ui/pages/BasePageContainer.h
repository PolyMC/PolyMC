#pragma once

class BasePageContainer
{
public:
    virtual ~BasePageContainer() = default;
    virtual bool selectPage(QString pageId) = 0;
    virtual void refreshContainer() = 0;
    virtual bool requestClose() = 0;
};
