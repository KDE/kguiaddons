// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <kguiaddons_export.h>

#include <QImage>

/**
 * @brief Decodes image to and from the BlurHash format. See https://blurha.sh/.
 *
 * @since 6.7.0
 */
class KGUIADDONS_EXPORT KBlurHash
{
public:
    /** Decodes the @p blurhash string creating an image of @p size.
     * @note Returns a null image if decoding failed.
     */
    static QImage decode(const QStringView &blurhash, const QSize &size);

protected:
    struct Components {
        int x, y;

        bool operator==(const Components &other) const
        {
            return x == other.x && y == other.y;
        }
    };

    /**
     * @brief Decodes a base 83 string to it's integer value. Returns std::nullopt if there's an invalid character in the blurhash.
     */
    static std::optional<int> decode83(const QStringView &encodedString);

    /**
     * @brief Unpacks an integer to it's @c Components value.
     */
    static Components unpackComponents(int packedComponents);

    /**
     * @brief Decodes a encoded max AC component value.
     */
    static float decodeMaxAC(int value);

    /**
     * @brief Decodes the average color from the encoded RGB value.
     * @note This returns the color as SRGB.
     */
    static QColor decodeAverageColor(int encodedValue);

    /**
     * @brief Calls pow() with @p exp on @p value, while keeping the sign.
     */
    static float signPow(float value, float exp);

    /**
     * @brief Decodes a encoded AC component value.
     */
    static QColor decodeAC(int value, float maxAC);

    /**
     * @brief Calculates the weighted sum for @p dimension across @p components.
     */
    static QList<float> calculateWeights(qsizetype dimension, qsizetype components);

    friend class KBlurHashTest;
};