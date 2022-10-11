/*
 * SPDX-FileCopyrightText: 2022 Georg Gadinger <nilsding@nilsding.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kcolorschemewatcher_mac.h"

#import <AppKit/AppKit.h>

#include <QTimer>

KColorSchemeWatcherMac::KColorSchemeWatcherMac()
{
    // subscribe to the distributed notification centre
    id notificationCenter = NSDistributedNotificationCenter.defaultCenter;
    m_observer = [notificationCenter addObserverForName:@"AppleInterfaceThemeChangedNotification"
                                                 object:nil
                                                  queue:nil
                                             usingBlock:^(NSNotification *) {
                                                 // "fun" workaround to not emit the signal immediately after receiving the notification.
                                                 // for some reason NSAppearance.currentDrawingAppearance is still set to the old value here, after a short
                                                 // delay it is updated correctly
                                                 QTimer::singleShot(0, [this]() {
                                                     Q_EMIT systemPreferenceChanged();
                                                 });
                                             }];
}

KColorSchemeWatcherMac::~KColorSchemeWatcherMac()
{
    [NSDistributedNotificationCenter.defaultCenter removeObserver:static_cast<id>(m_observer)];
}

KColorSchemeWatcher::ColorPreference KColorSchemeWatcherMac::systemPreference() const
{
    return NSAppearance.currentDrawingAppearance.name == NSAppearanceNameDarkAqua ? KColorSchemeWatcher::PreferDark : KColorSchemeWatcher::PreferLight;
}
