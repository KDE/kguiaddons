/*
 * SPDX-FileCopyrightText: 2022 Georg Gadinger <nilsding@nilsding.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kcolorschemewatcher_mac.h"

#import <AppKit/AppKit.h>

KColorSchemeWatcherMac::KColorSchemeWatcherMac()
{
    // subscribe to the distributed notification centre
    id notificationCenter = NSDistributedNotificationCenter.defaultCenter;
    m_observer = [notificationCenter addObserverForName:@"AppleInterfaceThemeChangedNotification"
                                                 object:nil
                                                  queue:nil
                                             usingBlock:^(NSNotification *) { Q_EMIT systemPreferenceChanged(); }];
}

KColorSchemeWatcherMac::~KColorSchemeWatcherMac()
{
    [NSDistributedNotificationCenter.defaultCenter removeObserver:static_cast<id>(m_observer)];
}

KColorSchemeWatcher::ColorPreference KColorSchemeWatcherMac::systemPreference() const
{
    return NSAppearance.currentDrawingAppearance.name == NSAppearanceNameDarkAqua ? KColorSchemeWatcher::PreferDark : KColorSchemeWatcher::PreferLight;
}
