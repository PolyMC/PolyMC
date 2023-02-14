#include "Common.h"

#include <QFontMetrics>

// Origin: Qt
// More specifically, this is a trimmed down version on the algorithm in:
// https://code.woboq.org/qt5/qtbase/src/widgets/styles/qcommonstyle.cpp.html#846
QStringList viewItemTextLayout(QTextLayout& textLayout, QSize bounds, qreal& height)
{
    QStringList result;
    height = 0;

    QFontMetrics fontMetrics{ textLayout.font() };

    textLayout.beginLayout();

    QString str = textLayout.text();
    while (true) {
        QTextLine line = textLayout.createLine();

        if (!line.isValid())
            break;
        if (line.textLength() == 0)
            break;

        line.setLineWidth(bounds.width());
        height += line.height();

        // If the *next* line has enough space to be drawn, then we don't need to elide this line.
        if (height + fontMetrics.lineSpacing() < bounds.height()) {
            result.append(str.mid(line.textStart(), line.textLength()));
        } else {
            // Otherwise, if *this* line has enough space to be drawn, draw it elided.
            if (height < bounds.height()) {
                result.append(fontMetrics.elidedText(str.mid(line.textStart()), Qt::ElideRight, bounds.width()));
            }
            // And end it here, since we know there's not enough space to draw the next line.
            break;
        }
    }

    textLayout.endLayout();

    return result;
}
