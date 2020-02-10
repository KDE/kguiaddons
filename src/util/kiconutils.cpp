/*
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kiconutils.h"

#include <QIconEngine>
#include <QPainter>
#include <QHash>

class KOverlayIconEngine : public QIconEngine
{
public:
    KOverlayIconEngine(const QIcon &icon, const QIcon &overlay, Qt::Corner position);
    KOverlayIconEngine(const QIcon &icon, const QHash<Qt::Corner, QIcon> &overlays);
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine *clone() const override;

    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;

    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state) override;
    void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state) override;

private:
    QIcon m_base;
    QHash<Qt::Corner, QIcon> m_overlays;
};

KOverlayIconEngine::KOverlayIconEngine(const QIcon &icon, const QIcon &overlay, Qt::Corner position)
    : QIconEngine()
{
    m_base = icon;
    m_overlays.insert(position, overlay);
}

KOverlayIconEngine::KOverlayIconEngine(const QIcon &icon, const QHash<Qt::Corner, QIcon> &overlays)
    : QIconEngine()
{
    m_base = icon;
    m_overlays = overlays;
}

QIconEngine *KOverlayIconEngine::clone() const
{
    return new KOverlayIconEngine(*this);
}

QSize KOverlayIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    return m_base.actualSize(size, mode, state);
}

QPixmap KOverlayIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);

    paint(&p, pixmap.rect(), mode, state);

    return pixmap;
}

void KOverlayIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state)
{
    m_base.addPixmap(pixmap, mode, state);
}

void KOverlayIconEngine::addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    m_base.addFile(fileName, size, mode, state);
}

void KOverlayIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    // Paint the base icon as the first layer
    m_base.paint(painter, rect, Qt::AlignCenter, mode, state);

    if (m_overlays.isEmpty()) {
        return;
    }

    const int width = rect.width();
    const int height = rect.height();
    const int iconSize = qMin(width, height);
    // Determine the overlay icon size
    int overlaySize;
    if (iconSize < 32) {
        overlaySize = 8;
    } else if (iconSize <= 48) {
        overlaySize = 16;
    } else if (iconSize <= 64) {
        overlaySize = 22;
    } else if (iconSize <= 96) {
        overlaySize = 32;
    } else if (iconSize <= 128) {
        overlaySize = 48;
    } else {
        overlaySize = (int)(iconSize / 4);
    }

    // Iterate over stored overlays
    QHash<Qt::Corner, QIcon>::const_iterator i = m_overlays.constBegin();
    while (i != m_overlays.constEnd()) {

        const QPixmap overlayPixmap = i.value().pixmap(overlaySize, overlaySize, mode, state);
        if (overlayPixmap.isNull()) {
            ++i;
            continue;
        }

        QPoint startPoint;
        switch (i.key()) {
        case Qt::BottomLeftCorner:
            startPoint = QPoint(2, height - overlaySize - 2);
            break;
        case Qt::BottomRightCorner:
            startPoint = QPoint(width - overlaySize - 2, height - overlaySize - 2);
            break;
        case Qt::TopRightCorner:
            startPoint = QPoint(width - overlaySize - 2, 2);
            break;
        case Qt::TopLeftCorner:
            startPoint = QPoint(2, 2);
            break;
        }

        // Draw the overlay pixmap
        painter->drawPixmap(startPoint, overlayPixmap);

        ++i;
    }
}

// ============================================================================

namespace KIconUtils
{

QIcon addOverlay(const QIcon &icon, const QIcon &overlay, Qt::Corner position)
{
    return QIcon(new KOverlayIconEngine(icon, overlay, position));
}

QIcon addOverlays(const QIcon &icon, const QHash<Qt::Corner, QIcon> &overlays)
{
    return QIcon(new KOverlayIconEngine(icon, overlays));
}

}
