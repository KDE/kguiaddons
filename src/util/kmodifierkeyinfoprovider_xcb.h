/*
    SPDX-FileCopyrightText: 2009 Michael Leupold <lemma@confuego.org>
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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
    ~KModifierKeyInfoProviderXcb() override;

    struct KeyMask {
        Qt::Key xkbKey = Qt::Key_unknown;
        unsigned int mask = 0;
    };

    using KeyMaskIterator = std::vector<KeyMask>::const_iterator;

    KeyMaskIterator findKey(Qt::Key key) const;

    bool setKeyLatched(Qt::Key key, bool latched) override;
    bool setKeyLocked(Qt::Key key, bool locked) override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override;
#endif

    void xkbUpdateModifierMapping();
    void xkbModifierStateChanged(unsigned char mods, unsigned char latched_mods, unsigned char locked_mods);
    void xkbButtonStateChanged(unsigned short ptr_buttons);

private:
    int m_xkbEv;
    bool m_xkbAvailable;

    std::vector<KeyMask> m_xkbModifiers;

    // maps a Qt::MouseButton to a button mask
    QHash<Qt::MouseButton, unsigned short> m_xkbButtons;
};

#endif
