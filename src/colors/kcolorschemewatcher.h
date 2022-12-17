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
    enum ColorPreference {
        NoPreference = 0, /** No preference available */
        PreferDark, /** The user prefers a dark color scheme */
        PreferLight, /** The user prefers a light color scheme */
    };
    Q_ENUM(ColorPreference)

    /**
     * @since 5.102
     */
    enum PreferenceType {
        AppsColorPreference = 0, /** The returned color preference is for apps. This is the default. */
        SystemColorPreference /** The returned color preference is for the system itself. Currently only supported on Windows */
    };
    Q_ENUM(PreferenceType)

    KColorSchemeWatcher(PreferenceType type, QObject *parent = nullptr);
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
