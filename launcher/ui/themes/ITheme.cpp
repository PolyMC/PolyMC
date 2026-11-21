#include "ITheme.h"
#include "rainbow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "Application.h"

namespace
{
void blend(QPalette& palette, qreal bias, QColor color,
           QPalette::ColorRole role)
{
    QColor from = palette.color(QPalette::Active, role);
    QColor blended = Rainbow::mix(from, color, bias);
    palette.setColor(QPalette::Disabled, role, blended);
}
}  // namespace

void ITheme::apply(bool)
{
    QApplication::setStyle(QStyleFactory::create(qtTheme()));
    if (hasColorScheme())
    {
        QApplication::setPalette(colorScheme());
    }
    if (hasStyleSheet())
    {
        APPLICATION->setStyleSheet(appStyleSheet());
    }
    else
    {
        APPLICATION->setStyleSheet(QString());
    }
    QDir::setSearchPaths("theme", searchPaths());
}

QPalette ITheme::fadeInactive(QPalette in, qreal bias, QColor color)
{
    blend(in, bias, color, QPalette::Window);
    blend(in, bias, color, QPalette::WindowText);
    blend(in, bias, color, QPalette::Base);
    blend(in, bias, color, QPalette::AlternateBase);
    blend(in, bias, color, QPalette::ToolTipBase);
    blend(in, bias, color, QPalette::ToolTipText);
    blend(in, bias, color, QPalette::Text);
    blend(in, bias, color, QPalette::Button);
    blend(in, bias, color, QPalette::ButtonText);
    blend(in, bias, color, QPalette::BrightText);
    blend(in, bias, color, QPalette::Link);
    blend(in, bias, color, QPalette::Highlight);
    blend(in, bias, color, QPalette::HighlightedText);
    return in;
}
