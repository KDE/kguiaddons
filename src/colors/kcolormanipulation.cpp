/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Janet Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QColorSpace>
#include <QtMath>
#include <math.h>

#include "kcolormanipulation.h"

namespace
{
qreal cubeRootOf(qreal num)
{
    return qPow(num, 1.0 / 3.0);
}

qreal cubed(qreal num)
{
    return num * num * num;
}
};

struct LAB {
    qreal L = 0;
    qreal a = 0;
    qreal b = 0;
};

// precomputed matrices from Björn Ottosson, public domain. or MIT if your country doesn't do that.
/*
    SPDX-FileCopyrightText: 2020 Björn Ottosson

    SPDX-License-Identifier: MIT
    SPDX-License-Identifier: None
*/

LAB linearSRGBToOKLab(const QColor &c)
{
    // convert from srgb to linear lms

    const auto l = 0.4122214708 * c.redF() + 0.5363325363 * c.greenF() + 0.0514459929 * c.blueF();
    const auto m = 0.2119034982 * c.redF() + 0.6806995451 * c.greenF() + 0.1073969566 * c.blueF();
    const auto s = 0.0883024619 * c.redF() + 0.2817188376 * c.greenF() + 0.6299787005 * c.blueF();

    // convert from linear lms to non-linear lms

    const auto l_ = cubeRootOf(l);
    const auto m_ = cubeRootOf(m);
    const auto s_ = cubeRootOf(s);

    // convert from non-linear lms to lab

    return LAB{.L = 0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_,
               .a = 1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_,
               .b = 0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_};
}

QColor OKLabToLinearSRGB(LAB lab)
{
    // convert from lab to non-linear lms

    const auto l_ = lab.L + 0.3963377774 * lab.a + 0.2158037573 * lab.b;
    const auto m_ = lab.L - 0.1055613458 * lab.a - 0.0638541728 * lab.b;
    const auto s_ = lab.L - 0.0894841775 * lab.a - 1.2914855480 * lab.b;

    // convert from non-linear lms to linear lms

    const auto l = cubed(l_);
    const auto m = cubed(m_);
    const auto s = cubed(s_);

    // convert from linear lms to linear srgb

    const auto r = +4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s;
    const auto g = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s;
    const auto b = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s;

    return QColor::fromRgbF(r, g, b);
}

auto toLinearSRGB = QColorSpace(QColorSpace::SRgb).transformationToColorSpace(QColorSpace::SRgbLinear);
auto fromLinearSRGB = QColorSpace(QColorSpace::SRgbLinear).transformationToColorSpace(QColorSpace::SRgb);

QColor KColorManipulation::lighter(const QColor &color, qreal factor)
{
    auto lab = linearSRGBToOKLab(toLinearSRGB.map(color));
    lab.L = qBound(0.0, lab.L * (1.0 + factor), 1.0);
    return fromLinearSRGB.map(OKLabToLinearSRGB(lab));
}

QColor KColorManipulation::darker(const QColor &color, qreal factor)
{
    auto lab = linearSRGBToOKLab(toLinearSRGB.map(color));
    lab.L = qBound(0.0, lab.L * (1.0 - factor), 1.0);
    return fromLinearSRGB.map(OKLabToLinearSRGB(lab));
}

QColor KColorManipulation::adjustedLightness(const QColor &color, qreal factor)
{
    auto lab = linearSRGBToOKLab(toLinearSRGB.map(color));
    lab.L = qBound(0.0, lab.L * factor, 1.0);
    return fromLinearSRGB.map(OKLabToLinearSRGB(lab));
}

QColor KColorManipulation::tint(const QColor &base, const QColor &with, qreal factor)
{
    auto baseLAB = linearSRGBToOKLab(toLinearSRGB.map(base));
    const auto withLAB = linearSRGBToOKLab(toLinearSRGB.map(with));
    baseLAB.a = ::lerp(baseLAB.a, withLAB.a, factor);
    baseLAB.b = ::lerp(baseLAB.b, withLAB.b, factor);

    return fromLinearSRGB.map(OKLabToLinearSRGB(baseLAB));
}

QColor KColorManipulation::lerp(const QColor &c1, const QColor &c2, qreal bias)
{
    if (bias <= 0.0) {
        return c1;
    }
    if (bias >= 1.0) {
        return c2;
    }

    const auto c1Linear = toLinearSRGB.map(c1);
    const auto c2Linear = toLinearSRGB.map(c2);

    qreal a = ::lerp(c1Linear.alphaF(), c2Linear.alphaF(), bias);
    if (a <= 0.0) {
        return Qt::transparent;
    }

    qreal r = qBound(0.0, ::lerp(c1Linear.redF() * c1Linear.alphaF(), c2Linear.redF() * c2Linear.alphaF(), bias), 1.0) / a;
    qreal g = qBound(0.0, ::lerp(c1Linear.greenF() * c1Linear.alphaF(), c2Linear.greenF() * c2Linear.alphaF(), bias), 1.0) / a;
    qreal b = qBound(0.0, ::lerp(c1Linear.blueF() * c1Linear.alphaF(), c2Linear.blueF() * c2Linear.alphaF(), bias), 1.0) / a;

    return fromLinearSRGB.map(QColor::fromRgbF(r, g, b, a));
}

QColor KColorManipulation::adjustedSaturation(const QColor &base, qreal factor)
{
    auto baseLAB = linearSRGBToOKLab(toLinearSRGB.map(base));

    const auto midpoint = 0;
    baseLAB.a = ::lerp(baseLAB.a, midpoint, factor);
    baseLAB.b = ::lerp(baseLAB.b, midpoint, factor);

    return fromLinearSRGB.map(OKLabToLinearSRGB(baseLAB));
}
