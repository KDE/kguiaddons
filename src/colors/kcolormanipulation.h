/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Janet Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORMANIPULATION_H
#define KCOLORMANIPULATION_H

#include <QColor>
#include <kguiaddons_export.h>

/*!
 *
 * Color manipulation utilities based on OKLAB
 *
 * \since 6.23
 */
namespace KColorManipulation
{

/*!
 * Lightens a color.
 *
 * \a color the color to manipulate.
 * \a factor factor by which to adjust the lightness. factor == 1.0 doubles the lightness, factor == -0.5 halves the lightness.
 * \sa KColorManipulation::adjustedLightness
 */
KGUIADDONS_EXPORT QColor lighter(const QColor &color, qreal factor);
/*!
 * Darkens a color.
 *
 * \a color the color to manipulate.
 * \a factor factor by which to adjust the lightness. factor == 1.0 halves the lightness, factor == -0.5 doubles the lightness.
 * \sa KColorManipulation::adjustedLightness
 */
KGUIADDONS_EXPORT QColor darker(const QColor &color, qreal factor);
/*!
 * Adjusts the lightness of a color.
 *
 * \a color the color to manipulate.
 * \a factor factor by which to adjust the lightness. lightness is multiplied by factor.
 * \sa KColorManipulation::lighter
 * \sa KColorManipulation::darker
 */
KGUIADDONS_EXPORT QColor adjustedLightness(const QColor &color, qreal factor);
/*!
 * Tints a color with another color.
 *
 * \a base the color to be tinted
 * \a with the color to tint base with
 * \a factor the factor by which to tint. 0.0 returns base, 1.0 returns base's lightness with with's hue.
 */
KGUIADDONS_EXPORT QColor tint(const QColor &base, const QColor &with, qreal factor);
/*!
 * Adjusts a color's saturation by bringing its A and B channels closer towards the midpoint or further away from it.
 *
 * \a color the color to de/saturate
 * \a factor the factor by which to desaturate. 0.0 returns color, 1.0 returns the most desaturated color, and negative values saturate the color
 */
KGUIADDONS_EXPORT QColor adjustedSaturation(const QColor &base, qreal factor);
/*!
 * Linearly interpolates between two colors.
 *
 * \a c1 the color to interpolate from
 * \a c2 the color to interpolate to
 * \a bias the bias to interpolate with. 0.0 returns c1, 1.0 returns c2.
 */
KGUIADDONS_EXPORT QColor lerp(const QColor &c1, const QColor &c2, qreal bias);

}

#endif // KCOLORMANIPULATION_H
