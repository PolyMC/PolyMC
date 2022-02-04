#pragma once

#include "ITheme.h"

class FusionTheme: public ITheme
{
public:
    ~FusionTheme() override {}

    QString qtTheme() override;
};
