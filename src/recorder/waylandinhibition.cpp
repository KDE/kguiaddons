/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
*/

#include "waylandinhibition_p.h"

#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <QtWaylandClient/QWaylandClientExtensionTemplate>

#include "qwayland-keyboard-shortcuts-inhibit-unstable-v1.h"

class ShortcutsInhibitManager
    : public QWaylandClientExtensionTemplate<ShortcutsInhibitManager>
    , public QtWayland::zwp_keyboard_shortcuts_inhibit_manager_v1
{
public:
    ShortcutsInhibitManager()
        : QWaylandClientExtensionTemplate<ShortcutsInhibitManager>(1)
    {
    }
    ~ShortcutsInhibitManager()
    {
        destroy();
    }
};

class ShortcutsInhibitor : public QtWayland::zwp_keyboard_shortcuts_inhibitor_v1
{
public:
    ShortcutsInhibitor(::zwp_keyboard_shortcuts_inhibitor_v1 *id)
        : QtWayland::zwp_keyboard_shortcuts_inhibitor_v1(id)
    {
    }

    ~ShortcutsInhibitor()
    {
        destroy();
    }

    void zwp_keyboard_shortcuts_inhibitor_v1_active() override
    {
        m_active = true;
    }

    void zwp_keyboard_shortcuts_inhibitor_v1_inactive() override
    {
        m_active = false;
    }

    bool isActive() const 
    {
        return m_active;
    }

private:
    bool m_active = false;
};

WaylandInhibition::WaylandInhibition(QWindow *window)
    : ShortcutInhibition()
    , m_manager(new ShortcutsInhibitManager)
    , m_window(window)
{
}

WaylandInhibition::~WaylandInhibition() = default;

bool WaylandInhibition::shortcutsAreInhibited() const
{
    return m_inhibitor && m_inhibitor->isActive();
}

void WaylandInhibition::enableInhibition()
{
    if (m_inhibitor || !m_manager->isActive()) {
        return;
    }
    QPlatformNativeInterface *nativeInterface = qApp->platformNativeInterface();
    if (!nativeInterface) {
        return;
    }
    auto seat = static_cast<wl_seat*>(nativeInterface->nativeResourceForIntegration("wl_seat"));
    auto surface = static_cast<wl_surface*>(nativeInterface->nativeResourceForWindow("surface", m_window));
    if (!seat || !surface) {
        return;
    }
    m_inhibitor.reset(new ShortcutsInhibitor(m_manager->inhibit_shortcuts(surface, seat)));
}

void WaylandInhibition::disableInhibition()
{
    if (!m_inhibitor) {
        return;
    }
    m_inhibitor.reset();
}

