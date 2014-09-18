/*********************************************************************************
 *                                                                               *
 * Copyright (C) 2005, 2009, 2014 by Albert Astals Cid <aacid@kde.org>           *
 *                                                                               *
 * This library is free software; you can redistribute it and/or                 *
 * modify it under the terms of the GNU Lesser General Public                    *
 * License as published by the Free Software Foundation; either                  *
 * version 2.1 of the License, or (at your option) version 3, or any             *
 * later version accepted by the membership of KDE e.V. (or its                  *
 * successor approved by the membership of KDE e.V.), which shall                *
 * act as a proxy defined in Section 6 of version 3 of the license.              *
 *                                                                               *
 * This library is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
 * Lesser General Public License for more details.                               *
 *                                                                               *
 * You should have received a copy of the GNU Lesser General Public              *
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                               *
 *********************************************************************************/

#include "kfontutils.h"

#include <qmath.h>
#include <qpainter.h>

static bool checkFits(QPainter &painter, const QString &string, qreal width, qreal height, qreal size, KFontUtils::AdaptFontSizeOptions flags)
{
    QFont f = painter.font();
    f.setPointSizeF(size);
    painter.setFont(f);
    int qtFlags = Qt::AlignCenter | Qt::TextWordWrap;
    if (flags & KFontUtils::DoNotAllowWordWrap) {
        qtFlags &= ~Qt::TextWordWrap;
    }
    const QRectF boundingRect = painter.boundingRect(QRectF(0, 0, width, height), qtFlags, string);
    if (boundingRect.width() == 0 || boundingRect.height() == 0) {
        return false;
    } else if (boundingRect.width() > width || boundingRect.height() > height) {
        return false;
    }
    return true;
}

qreal KFontUtils::adaptFontSize(QPainter &painter, const QString &string, qreal width, qreal height, qreal maxFontSize, qreal minFontSize, AdaptFontSizeOptions flags)
{
    // A invalid range is an error (-1)
    if (maxFontSize < minFontSize)
        return -1;

    // If the max font size already fits, return it
    if (checkFits(painter, string, width, height, maxFontSize, flags))
        return maxFontSize;

    qreal fontSizeDoesNotFit = maxFontSize;

    // If the min font size does not fit, try to see if a font size of 1 fits,
    // if it does not return error (-1)
    // if it does, we'll return a fontsize smaller than the minFontSize as documented
    if (!checkFits(painter, string, width, height, minFontSize, flags)) {
        fontSizeDoesNotFit = minFontSize;

        minFontSize = 1;
        if (!checkFits(painter, string, width, height, minFontSize, flags))
            return -1;
    }

    qreal fontSizeFits = minFontSize;
    qreal nextFontSizeToTry = (fontSizeDoesNotFit + fontSizeFits) / 2;

    while (qFloor(fontSizeFits) != qFloor(nextFontSizeToTry)) {
        if (checkFits(painter, string, width, height, nextFontSizeToTry, flags)) {
            fontSizeFits = nextFontSizeToTry;
            nextFontSizeToTry = (fontSizeDoesNotFit + fontSizeFits) / 2;
        } else {
            fontSizeDoesNotFit = nextFontSizeToTry;
            nextFontSizeToTry = (nextFontSizeToTry + fontSizeFits) / 2;
        }
    }

    QFont f = painter.font();
    f.setPointSizeF(fontSizeFits);
    painter.setFont(f);

    return fontSizeFits;
}

qreal KFontUtils::adaptFontSize(QPainter &painter, const QString &text, const QSizeF &availableSize, qreal maxFontSize, qreal minFontSize, AdaptFontSizeOptions flags)
{
    return adaptFontSize(painter, text, availableSize.width(), availableSize.height(), maxFontSize, minFontSize, flags);
}

