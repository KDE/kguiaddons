/*
    SPDX-FileCopyrightText: 2009 Michael Leupold <lemma@confuego.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kmodifierkeyinfoprovider_p.h"

KModifierKeyInfoProvider::KModifierKeyInfoProvider()
    : QObject(nullptr)
{
}

KModifierKeyInfoProvider::~KModifierKeyInfoProvider() = default;

bool KModifierKeyInfoProvider::setKeyLatched(Qt::Key key, bool latched)
{
    Q_UNUSED(key);
    Q_UNUSED(latched);
    return false;
}

bool KModifierKeyInfoProvider::setKeyLocked(Qt::Key key, bool locked)
{
    Q_UNUSED(key);
    Q_UNUSED(locked);
    return false;
}

bool KModifierKeyInfoProvider::isFlagSet(Qt::Key key, ModifierFlag flag) const
{
    auto it = std::find_if(m_modifierStates.cbegin(), m_modifierStates.cend(), [key](const ModifierKeyInfo &info) {
        return info.modKey == key;
    });

    return it != m_modifierStates.cend() ? it->modFlags & flag : false;
}

bool KModifierKeyInfoProvider::isKeyPressed(Qt::Key key) const
{
    return isFlagSet(key, Pressed);
}

bool KModifierKeyInfoProvider::isKeyLatched(Qt::Key key) const
{
    return isFlagSet(key, Latched);
}

bool KModifierKeyInfoProvider::isKeyLocked(Qt::Key key) const
{
    return isFlagSet(key, Locked);
}

bool KModifierKeyInfoProvider::isButtonPressed(Qt::MouseButton button) const
{
    auto it = std::find_if(m_MouseBtnStates.cbegin(), m_MouseBtnStates.cend(), [button](const MouseButtonInfo &info) {
        return info.mouseButton == button;
    });

    return it != m_MouseBtnStates.cend() ? it->state : false;
}

bool KModifierKeyInfoProvider::knowsKey(Qt::Key key) const
{
    auto it = std::find_if(m_modifierStates.cbegin(), m_modifierStates.cend(), [key](const ModifierKeyInfo &info) {
        return info.modKey == key;
    });

    return it != m_modifierStates.cend();
}

const QList<Qt::Key> KModifierKeyInfoProvider::knownKeys() const
{
    QList<Qt::Key> list;
    list.reserve(m_modifierStates.size());
    std::transform(m_modifierStates.cbegin(), m_modifierStates.cend(), std::back_inserter(list), [](const ModifierKeyInfo &info) {
        return info.modKey;
    });
    return list;
}

void KModifierKeyInfoProvider::stateUpdated(const ModifierKeyInfo &newInfo)
{
    auto key = newInfo.modKey;
    auto newFlags = newInfo.modFlags;

    auto it = std::find_if(m_modifierStates.begin(), m_modifierStates.end(), [key](const ModifierKeyInfo &info) {
        return info.modKey == key;
    });

    Q_ASSERT(it != m_modifierStates.cend());

    auto &flags = it->modFlags;

    if (newFlags != flags) {
        const auto difference = (newFlags ^ flags);
        flags = newFlags;
        if (difference & Pressed) {
            Q_EMIT keyPressed(key, newFlags & Pressed);
        }
        if (difference & Latched) {
            Q_EMIT keyLatched(key, newFlags & Latched);
        }
        if (difference & Locked) {
            Q_EMIT keyLocked(key, newFlags & Locked);
        }
    }
}
