/*
    Copyright 2009  Michael Leupold <lemma@confuego.org>
    Copyright 2013  Martin Gräßlin <mgraesslin@kde.org>

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

#ifndef KMODIFIERKEYINFOPROVIDERXCB_H
#define KMODIFIERKEYINFOPROVIDERXCB_H

#include "kmodifierkeyinfoprovider_p.h"
#include <QAbstractNativeEventFilter>
class KModifierKeyInfoProviderXcb : public KModifierKeyInfoProvider, public QAbstractNativeEventFilter
{
Q_OBJECT
Q_PLUGIN_METADATA(IID "org.kde.kguiaddons.KModifierKeyInfoProvider.XCB")
public:
    KModifierKeyInfoProviderXcb();
    ~KModifierKeyInfoProviderXcb();

    bool setKeyLatched(Qt::Key key, bool latched) override;
    bool setKeyLocked(Qt::Key key, bool locked) override;

    bool nativeEventFilter(const QByteArray &eventType, void *message, long int *result) override;

    void xkbUpdateModifierMapping();
    void xkbModifierStateChanged(unsigned char mods, unsigned char latched_mods,
                                 unsigned char locked_mods);
    void xkbButtonStateChanged(unsigned short ptr_buttons);

private:
    int m_xkbEv;
    bool m_xkbAvailable;

    // maps a Qt::Key to a modifier mask
    QHash<Qt::Key, unsigned int> m_xkbModifiers;
    // maps a Qt::MouseButton to a button mask
    QHash<Qt::MouseButton, unsigned short> m_xkbButtons;
};

#endif
