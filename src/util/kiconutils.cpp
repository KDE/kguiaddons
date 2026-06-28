/*
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <array>

#include "kiconutils.h"

#include <QHash>
#include <QIconEngine>
#include <QPainter>

class KOverlayIconEngine : public QIconEngine
{
public:
    KOverlayIconEngine(const QIcon &icon, const QIcon &overlay, Qt::Corner position);
    KOverlayIconEngine(const QIcon &icon, const QHash<Qt::Corner, QIcon> &overlays);
    KOverlayIconEngine(const QIcon &icon, const QStringList &overlays);
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine *clone() const override;

    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;

    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state) override;
    void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state) override;

    void virtual_hook(int id, void *data) override;

private:
    QIcon m_base;
    QHash<Qt::Corner, QIcon> m_overlays;
};

KOverlayIconEngine::KOverlayIconEngine(const QIcon &icon, const QIcon &overlay, Qt::Corner position)
    : QIconEngine()
    , m_base(icon)
{
    m_overlays.insert(position, overlay);
}

KOverlayIconEngine::KOverlayIconEngine(const QIcon &icon, const QHash<Qt::Corner, QIcon> &overlays)
    : QIconEngine()
    , m_base(icon)
    , m_overlays(overlays)
{
}

KOverlayIconEngine::KOverlayIconEngine(const QIcon &icon, const QStringList &overlays)
    : QIconEngine()
    , m_base(icon)
{
    const std::array<Qt::Corner, 4> indexToCorner{
        Qt::BottomRightCorner,
        Qt::BottomLeftCorner,
        Qt::TopLeftCorner,
        Qt::TopRightCorner,
    };

    // static_cast becaue size() returns a qsizetype in Qt6
    const int count = std::min(4, static_cast<int>(overlays.size()));

    m_overlays.reserve(count);

    for (int i = 0; i < count; i++) {
        m_overlays.insert(indexToCorner[i], QIcon::fromTheme(overlays.at(i)));
    }
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

void KOverlayIconEngine::virtual_hook(int id, void *data)
{
    if (id == QIconEngine::ScaledPixmapHook) {
        auto *info = reinterpret_cast<ScaledPixmapArgument *>(data);

        // Since https://codereview.qt-project.org/c/qt/qtbase/+/563553 size is in logical pixels
        QSize phyiscalSize = info->size * info->scale;
        QPixmap pixmap(phyiscalSize);
        pixmap.setDevicePixelRatio(info->scale);
        pixmap.fill(Qt::transparent);

        const QRect logicalRect(0, 0, phyiscalSize.width() / info->scale, phyiscalSize.height() / info->scale);
        QPainter p(&pixmap);

        paint(&p, logicalRect, info->mode, info->state);

        info->pixmap = pixmap;

        return;
    }
    QIconEngine::virtual_hook(id, data);
}

void KOverlayIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    m_base.paint(painter, rect, Qt::AlignCenter, mode, state);

    if (m_overlays.isEmpty()) {
        return;
    }

    // Use the painter's device pixel ratio so the overlays are sharp regardless of
    // how the engine is painted (QIcon::paint(), pixmap() or the scaled-pixmap hook).
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;

    // Anchor the overlays to the base icon as it is actually painted (centered and
    // aspect-ratio-preserving within rect) rather than to rect, so they stay in the
    // corners of the icon for non-square or smaller-than-rect icons.
    QRect iconRect(QPoint(0, 0), m_base.actualSize(rect.size(), mode, state).boundedTo(rect.size()));
    iconRect.moveCenter(rect.center());

    const int left = iconRect.left();
    const int top = iconRect.top();
    const int width = iconRect.width();
    const int height = iconRect.height();
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
    // Never let the overlay be larger than the icon it is drawn on.
    overlaySize = qMin(overlaySize, iconSize);

    // Iterate over stored overlays
    QHash<Qt::Corner, QIcon>::const_iterator i = m_overlays.constBegin();
    while (i != m_overlays.constEnd()) {
        const QPixmap overlayPixmap = i.value().pixmap(QSize{overlaySize, overlaySize}, dpr, mode, state);
        if (overlayPixmap.isNull()) {
            ++i;
            continue;
        }

        QPoint startPoint;
        switch (i.key()) {
        case Qt::BottomLeftCorner:
            startPoint = QPoint(left + 2, top + height - overlaySize - 2);
            break;
        case Qt::BottomRightCorner:
            startPoint = QPoint(left + width - overlaySize - 2, top + height - overlaySize - 2);
            break;
        case Qt::TopRightCorner:
            startPoint = QPoint(left + width - overlaySize - 2, top + 2);
            break;
        case Qt::TopLeftCorner:
            startPoint = QPoint(left + 2, top + 2);
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

QIcon addOverlays(const QIcon &icon, const QStringList &overlays)
{
    if (overlays.count() == 0) {
        return icon;
    }

    return QIcon(new KOverlayIconEngine(icon, overlays));
}

QIcon addOverlays(const QString &iconName, const QStringList &overlays)
{
    const QIcon icon = QIcon::fromTheme(iconName);

    return addOverlays(icon, overlays);
}
}
