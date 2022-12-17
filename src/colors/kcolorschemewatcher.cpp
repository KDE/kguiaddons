/*
 * SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kcolorschemewatcher.h"

#include "kcolorschemewatcherbackend.h"

#ifdef Q_OS_WINDOWS
#include "kcolorschemewatcher_win.h"
#endif

#ifdef Q_OS_MACOS
#include "kcolorschemewatcher_mac.h"
#endif

#ifdef QT_DBUS_LIB
#include "kcolorschemewatcher_xdg.h"
#endif

class KColorSchemeWatcherPrivate
{
public:
    std::unique_ptr<KColorSchemeWatcherBackend> backend;

    KColorSchemeWatcherPrivate(KColorSchemeWatcher::PreferenceType type)
    {
#ifdef Q_OS_WINDOWS
        backend = std::make_unique<KColorSchemeWatcherWin>(type);
#elif defined(Q_OS_MACOS)
        if (type == KColorSchemeWatcher::AppsColorPreference) {
            backend = std::make_unique<KColorSchemeWatcherMac>();
        }
#elif defined(QT_DBUS_LIB)
        if (type == KColorSchemeWatcher::AppsColorPreference) {
            backend = std::make_unique<KColorSchemeWatcherXDG>();
        }
#endif
    }
};

KColorSchemeWatcher::KColorSchemeWatcher(PreferenceType type, QObject *parent)
    : QObject(parent)
    , d(new KColorSchemeWatcherPrivate(type))
{
    if (d->backend) {
        connect(d->backend.get(), &KColorSchemeWatcherBackend::systemPreferenceChanged, this, &KColorSchemeWatcher::systemPreferenceChanged);
    }
}

KColorSchemeWatcher::KColorSchemeWatcher(QObject *parent)
    : KColorSchemeWatcher(AppsColorPreference, parent)
{
}

KColorSchemeWatcher::~KColorSchemeWatcher()
{
}

KColorSchemeWatcher::ColorPreference KColorSchemeWatcher::systemPreference() const
{
    if (d->backend) {
        return d->backend->systemPreference();
    }

    return NoPreference;
}
