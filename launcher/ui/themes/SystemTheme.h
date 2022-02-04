#pragma once

#include "ITheme.h"

class SystemTheme: public ITheme
{
public:
    SystemTheme();
    ~SystemTheme() override {}
    void apply(bool initial) override;

    QString id() override;
    QString name() override;
    QString qtTheme() override;
    bool hasStyleSheet() override;
    QString appStyleSheet() override;
    bool hasColorScheme() override;
    QPalette colorScheme() override;
    double fadeAmount() override;
    QColor fadeColor() override;
private:
    QPalette systemPalette;
    QString systemTheme;
};
