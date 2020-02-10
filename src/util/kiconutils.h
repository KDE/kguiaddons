/*
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KICONUTILS_H
#define KICONUTILS_H

#include <kguiaddons_export.h>

#include <QIcon>

/**
 * @namespace KIconUtils
 * Provides utility functions for icons.
 */
namespace KIconUtils
{

/**
 * Adds the \a overlay over the \a icon in the specified \a position
 *
 * The \a overlay icon is scaled down approx. to 1/3 or 1/4 (depending on the icon size)
 * and placed in one of the corners of the base icon.
 */
KGUIADDONS_EXPORT QIcon addOverlay(const QIcon &icon, const QIcon &overlay, Qt::Corner position);

/**
 * Adds \a overlays over the \a icon
 *
 * The \a overlays is a QHash of Qt::Corner and QIcon. The Qt::Corner value
 * decides where the overlay icon will be painted, the QIcon value
 * is the overlay icon to be painted.
 *
 * The overlay icon is scaled down to 1/3 or 1/4 (depending on the icon size)
 * and placed in one of the corners of the base icon.
 */
KGUIADDONS_EXPORT QIcon addOverlays(const QIcon &icon, const QHash<Qt::Corner, QIcon> &overlays);
}

#endif // KICONUTILS_H
