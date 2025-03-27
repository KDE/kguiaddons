/*
 * SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KCOLORSCHEMEWATCHER_H
#define KCOLORSCHEMEWATCHER_H

#include "kguiaddons_export.h"

#include <QObject>

#include <memory>

class KColorSchemeWatcherPrivate;

/**
 * Information about system-wide color preferences.
 * @since 5.100
 */
class KGUIADDONS_EXPORT KColorSchemeWatcher : public QObject
{
    Q_OBJECT
public:
    /**
     * Encodes the color preference of the user to be used by applications
     * as configured in the system settings. On some systems not all values
     * are returned, e.g. @c PreferHighContrast is currently only returned
     * on Windows.
     * @see systemPreference
     */
    enum ColorPreference {
        NoPreference = 0, /** No preference available */
        PreferDark, /** The user prefers a dark color scheme */
        PreferLight, /** The user prefers a light color scheme */
        PreferHighContrast, /** The user prefers a system-provided high-contrast color scheme
                                @since 6.13 */
    };
    Q_ENUM(ColorPreference)

    KColorSchemeWatcher(QObject *parent = nullptr);
    ~KColorSchemeWatcher() override;

    /**
     * The system-wide color preference.
     */
    ColorPreference systemPreference() const;

Q_SIGNALS:
    /**
     * Emitted when systemPreference changes.
     */
    void systemPreferenceChanged();

private:
    std::unique_ptr<KColorSchemeWatcherPrivate> const d;
};

#endif
