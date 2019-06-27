/*
    Copyright 2009  Michael Leupold <lemma@confuego.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kmodifierkeyinfoprovider_p.h"

KModifierKeyInfoProvider::KModifierKeyInfoProvider()
    : QObject(nullptr)
{
}

KModifierKeyInfoProvider::~KModifierKeyInfoProvider()
{
}

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

bool KModifierKeyInfoProvider::isKeyPressed(Qt::Key key) const
{
    auto it = m_modifierStates.constFind(key);
    if (it != m_modifierStates.constEnd()) {
        return *it & Pressed;
    }
    return false;
}

bool KModifierKeyInfoProvider::isKeyLatched(Qt::Key key) const
{
    auto it = m_modifierStates.constFind(key);
    if (it != m_modifierStates.constEnd()) {
        return *it & Latched;
    }
    return false;
}

bool KModifierKeyInfoProvider::isKeyLocked(Qt::Key key) const
{
    auto it = m_modifierStates.constFind(key);
    if (it != m_modifierStates.constEnd()) {
        return *it & Locked;
    }
    return false;
}

bool KModifierKeyInfoProvider::isButtonPressed(Qt::MouseButton button) const
{
    if (m_buttonStates.contains(button)) {
        return m_buttonStates[button];
    }
    return false;
}

bool KModifierKeyInfoProvider::knowsKey(Qt::Key key) const
{
    return m_modifierStates.contains(key);
}

const QList<Qt::Key> KModifierKeyInfoProvider::knownKeys() const
{
    return m_modifierStates.keys();
}

void KModifierKeyInfoProvider::stateUpdated(Qt::Key key, KModifierKeyInfoProvider::ModifierStates newState)
{
    auto &state = m_modifierStates[key];
    if (newState != state) {
        const auto difference = (newState ^ state);
        state = newState;
        if (difference & Pressed) {
            emit keyPressed(key, newState & Pressed);
        }
        if (difference & Latched) {
            emit keyLatched(key, newState & Latched);
        }
        if (difference & Locked) {
            emit keyLocked(key, newState & Locked);
        }
    }
}
