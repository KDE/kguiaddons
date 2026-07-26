/*
    SPDX-FileCopyrightText: 2026 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kdragutils.h"

#include <QDrag>
#include <QGuiApplication>
#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QScreen>

namespace KDragUtils
{

static constexpr int dragPixmapIconExtent = 32; // TODO: better value? platform specific ones?
static constexpr int dragPixmapImageExtent = 192; // TODO: better value? platform specific ones?
static constexpr int dragPixmapOpacity = 178; // 70 % TODO: better value? platform specific ones?

bool boundDragPixmapSize(QSize &displaySize, DragObjectType dragobjectType)
{
    switch (dragobjectType) {
    case DragObjectType::CompleteObject: {
        if ((displaySize.height() > dragPixmapImageExtent) || (displaySize.width() > dragPixmapImageExtent)) {
            displaySize.scale(dragPixmapImageExtent, dragPixmapImageExtent, Qt::KeepAspectRatio);
            return true;
        }
        break;
    }
    case DragObjectType::Selection: {
        if (const QScreen *const screen = qApp->primaryScreen()) {
            const QSize halfScreenSize = screen->geometry().size() / 2;
            if ((displaySize.height() > halfScreenSize.height()) || (displaySize.width() > halfScreenSize.width())) {
                displaySize.scale(halfScreenSize, Qt::KeepAspectRatio);
                return true;
            }
        }
        break;
    }
    }
    return false;
}

bool setDragPixmap(QDrag *drag, DragPixmapPaintFunction paintFunction, QSize displaySize, QPoint dragOffset, DragObjectType dragobjectType)
{
    if (!drag) {
        return false;
    }

    QSize dragPixmapSize(displaySize);
    const bool isScaled = boundDragPixmapSize(dragPixmapSize, dragobjectType);

    // Use app's, not window's, dpr value to prepare drag pixmap
    // for being displayed on any screens during drag
    const qreal dpr = qApp->devicePixelRatio();

    QPixmap dragPixmap(dragPixmapSize * dpr);
    dragPixmap.setDevicePixelRatio(dpr);

    const bool paintSuccess = paintFunction(&dragPixmap, dragPixmapSize);
    if (!paintSuccess) {
        return false;
    }

    // make a tad transparent
    // reflects in-process state, allows to see through for targeting drop position
    // matches web browser behaviour
    QPainter painter(&dragPixmap);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    const QRect paintRect(QPoint(0, 0), dragPixmapSize);
    painter.fillRect(paintRect, QColor(0, 0, 0, dragPixmapOpacity));
    painter.end();

    drag->setPixmap(dragPixmap);

    // calculate drag hot spot
    QPoint hotSpot = dragOffset;
    if (isScaled) {
        const bool isLandscape = (displaySize.width() > displaySize.height());
        const qreal scale =
            isLandscape ? static_cast<qreal>(displaySize.width()) / dragPixmap.width() : static_cast<qreal>(displaySize.height()) / dragPixmap.height();
        hotSpot /= scale;
    }

    drag->setHotSpot(hotSpot);

    return true;
}

bool setDragPixmap(QDrag *drag, const QPixmap &pixmap, QSize displaySize, QPoint dragOffset, DragObjectType dragobjectType)
{
    auto painting = [pixmap](QPaintDevice *paintDevice, QSize pixmapSize) -> bool {
        QPainter painter(paintDevice);
        // copy scaled pixmap in, including alpha
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        const QRect paintRect(QPoint(0, 0), pixmapSize);
        painter.drawPixmap(paintRect, pixmap);
        return true;
    };

    return setDragPixmap(drag, painting, displaySize, dragOffset, dragobjectType);
}

bool setDragPixmap(QDrag *drag, const QImage &image, QSize displaySize, QPoint dragOffset, DragObjectType dragobjectType)
{
    auto painting = [image](QPaintDevice *paintDevice, QSize imageSize) -> bool {
        QPainter painter(paintDevice);
        // copy scaled pixmap in, including alpha
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        const QRect paintRect(QPoint(0, 0), imageSize);
        painter.drawImage(paintRect, image);
        return true;
    };

    return setDragPixmap(drag, painting, displaySize, dragOffset, dragobjectType);
}

bool setDragIcon(QDrag *drag, const QIcon &icon)
{
    if (icon.isNull()) {
        return false;
    }

    drag->setPixmap(icon.pixmap(dragPixmapIconExtent, dragPixmapIconExtent));

    return true;
}

}
