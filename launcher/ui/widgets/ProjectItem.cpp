#include "ProjectItem.h"

#include "Common.h"

#include <QIcon>
#include <QPainter>

#include <algorithm>

ProjectItemDelegate::ProjectItemDelegate(QWidget* parent) : QStyledItemDelegate(parent) {}

void ProjectItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    auto rect = opt.rect;

    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(rect, opt.palette.highlight());
        painter->setPen(opt.palette.highlightedText().color());
    } else if (opt.state & QStyle::State_MouseOver) {
        painter->fillRect(rect, opt.palette.window());
    }

    // The default icon size will be a square (and height is usually the lower value).
    auto icon_width = rect.height(), icon_height = rect.height();
    int icon_x_margin = (rect.height() - icon_width) / 2;
    int icon_y_margin = (rect.height() - icon_height) / 2;

    if (!opt.icon.isNull()) {  // Icon painting
        {
            auto icon_size = opt.decorationSize;
            icon_width = icon_size.width();
            icon_height = icon_size.height();

            float desired_dim = rect.height() - 10;

            auto scaleRatio = icon_width > icon_height ? desired_dim / icon_width : desired_dim / icon_height;

            icon_width *= scaleRatio;
            icon_height *= scaleRatio;

            icon_x_margin = (rect.height() - icon_width) / 2;
            icon_y_margin = (rect.height() - icon_height) / 2;
        }

        // Centralize icon with a margin to separate from the other elements
        int x = rect.x() + icon_x_margin;
        int y = rect.y() + icon_y_margin;

        // Prevent 'scaling null pixmap' warnings
        if (icon_width > 0 && icon_height > 0)
            opt.icon.paint(painter, x, y, icon_width, icon_height);
    }

    // Change the rect so that further painting is easier
    rect.setTopLeft(QPoint(rect.x() + icon_width + 2 * icon_x_margin, rect.y() + 4));

    {  // Title painting
        auto title = index.data(UserDataTypes::TITLE).toString();

        painter->save();

        auto font = opt.font;
        if (index.data(UserDataTypes::SELECTED).toBool()) {
            // Set nice font
            font.setBold(true);
            font.setUnderline(true);
        }

        font.setPointSize(font.pointSize() + 2);
        painter->setFont(font);

        QFontMetrics fontMetrics{font};

        QRect titleRect(rect.topLeft() + QPoint(0, fontMetrics.ascent() - fontMetrics.height()), QSize(rect.width(), fontMetrics.height()));
        // On the top, aligned to the left after the icon
        painter->drawText(titleRect, title, QTextOption(Qt::AlignTop));
        painter->restore();

        // Change the rect again so it takes up the space below the title text
        rect.setTop(titleRect.bottom());
    }

    {  // Description painting
        auto description = index.data(UserDataTypes::DESCRIPTION).toString();

        QTextLayout text_layout(description, opt.font);

        qreal height = 0;
        auto cut_text = viewItemTextLayout(text_layout, rect.size(), height);

        description = cut_text.join("\n");

        QRect descriptionRect = rect;
        painter->drawText(descriptionRect, Qt::TextWordWrap, description);
    }

    painter->restore();
}

QSize ProjectItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    int height = 0;

    // 2px spacing between top and title
    height += 2;
    {  // Ensure enough space for one line with the title font
        auto font = option.font;
        if (index.data(UserDataTypes::SELECTED).toBool()) {
            font.setBold(true);
            font.setUnderline(true);
        }

        font.setPointSize(font.pointSize() + 2);

        // Ensure enough space for the title
        height += QFontMetrics{ font }.height();
    }

    { // Ensure enough space for 2 lines of description text
        height += QFontMetrics{ option.font }.lineSpacing() * 2;
    }

    QSize indexSizeHint = index.data(Qt::SizeHintRole).toSize();

    if (indexSizeHint.isValid()) {
        return QSize(indexSizeHint.width(), std::max(indexSizeHint.height(), height));
    }

    return QSize(0, height);
}
