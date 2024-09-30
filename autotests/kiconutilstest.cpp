/*
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "kiconutilstest.h"
#include <kiconutils.h>

#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QTest>

QTEST_MAIN(KIconUtilsTest)

struct ImageInfo {
    int bluePixels = 0;
    int redPixels = 0;
    qreal dpr = 1;

    inline qreal dpr_squared()
    {
        return dpr * dpr;
    }
};

ImageInfo countRedBluePixels(const QImage &image)
{
    ImageInfo info;

    // Get the image DPR
    info.dpr = image.devicePixelRatio();
    qDebug() << "Image DPR:" << image.devicePixelRatio() << image.size();

    // Go over the image and count red and blue pixels
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (qRed(image.pixel(x, y)) == 255) {
                info.redPixels++;
            } else if (qBlue(image.pixel(x, y)) == 255) {
                info.bluePixels++;
            }
        }
    }

    return info;
}

void KIconUtilsTest::addOverlayTest()
{
    QPixmap rectanglePixmap(32, 32);
    rectanglePixmap.fill(Qt::red);

    QIcon icon(rectanglePixmap);

    QPixmap overlay(32, 32);
    overlay.fill(Qt::blue);

    QIcon overlayIcon(overlay);

    QIcon iconWithOverlay = KIconUtils::addOverlay(icon, overlayIcon, Qt::BottomRightCorner);
    QImage result = iconWithOverlay.pixmap(32, 32).toImage();

    auto info = countRedBluePixels(result);

    // For icon of size 32x32, the overlay should be 16x16 (=256)
    QCOMPARE(info.bluePixels, 256 * info.dpr_squared());
    QCOMPARE(info.redPixels, 768 * info.dpr_squared());

    // Try different size and position
    rectanglePixmap = rectanglePixmap.scaled(96, 96);

    icon = QIcon(rectanglePixmap);

    overlay = overlay.scaled(96, 96);

    overlayIcon = QIcon(overlay);

    iconWithOverlay = KIconUtils::addOverlay(icon, overlayIcon, Qt::BottomRightCorner);

    // Test if unsetting the overlay works;
    // the result should have just one blue square
    iconWithOverlay = KIconUtils::addOverlay(icon, QIcon(), Qt::BottomRightCorner);

    iconWithOverlay = KIconUtils::addOverlay(icon, overlayIcon, Qt::TopLeftCorner);
    result = iconWithOverlay.pixmap(96, 96).toImage();

    info = countRedBluePixels(result);

    // 96x96 big icon will have 32x32 big overlay (=1024 blue pixels)
    QCOMPARE(info.bluePixels, 1024 * info.dpr_squared());
    QCOMPARE(info.redPixels, 8192 * info.dpr_squared());

    // Try paint method
    icon = QIcon(rectanglePixmap);
    iconWithOverlay = KIconUtils::addOverlay(icon, overlayIcon, Qt::TopLeftCorner);

    QPixmap a(96, 96);
    QPainter p(&a);
    iconWithOverlay.paint(&p, a.rect(), Qt::AlignCenter, QIcon::Normal, QIcon::Off);

    result = a.toImage();

    info = countRedBluePixels(result);

    // 96x96 big icon will have 32x32 big overlay (=1024 blue pixels)
    QCOMPARE(info.bluePixels, 1024 * info.dpr_squared());
    QCOMPARE(info.redPixels, 8192 * info.dpr_squared());
}

void KIconUtilsTest::addOverlaysTest()
{
    QPixmap rectanglePixmap(32, 32);
    rectanglePixmap.fill(Qt::red);

    QIcon icon(rectanglePixmap);

    QPixmap overlay(32, 32);
    overlay.fill(Qt::blue);

    QIcon overlayIcon(overlay);

    QHash<Qt::Corner, QIcon> overlays;
    overlays.insert(Qt::BottomRightCorner, overlayIcon);
    overlays.insert(Qt::TopLeftCorner, overlayIcon);

    QIcon iconWithOverlay = KIconUtils::addOverlays(icon, overlays);
    QImage result = iconWithOverlay.pixmap(32, 32).toImage();

    auto info = countRedBluePixels(result);

    // Two blue overlays in icon size 32x32 would intersect with 16 pixels,
    // so the amount of blue pixels should be 2x256-16 = 496
    QCOMPARE(info.bluePixels, 496 * info.dpr_squared());
    QCOMPARE(info.redPixels, 528 * info.dpr_squared());

    // Try different size

    rectanglePixmap = rectanglePixmap.scaled(96, 96);
    icon = QIcon(rectanglePixmap);
    overlay = overlay.scaled(96, 96);
    overlayIcon = QIcon(overlay);

    // Clear the old sizes first
    overlays.clear();
    overlays.insert(Qt::BottomRightCorner, overlayIcon);
    overlays.insert(Qt::TopRightCorner, overlayIcon);
    overlays.insert(Qt::TopLeftCorner, overlayIcon);

    // Now it will have 3 overlays
    iconWithOverlay = KIconUtils::addOverlays(icon, overlays);

    QPixmap a(96, 96);
    QPainter p(&a);
    iconWithOverlay.paint(&p, a.rect(), Qt::AlignCenter, QIcon::Normal, QIcon::Off);

    result = a.toImage();

    info = countRedBluePixels(result);

    // 96x96 big icon will have 32x32 big overlays (=3072 blue pixels)
    QCOMPARE(info.bluePixels, 3072 * info.dpr_squared());
    QCOMPARE(info.redPixels, 6144 * info.dpr_squared());
}

#include "moc_kiconutilstest.cpp"
