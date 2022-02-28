/*
    SPDX-FileCopyrightText: 2009 Michael Leupold <lemma@confuego.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kmodifierkeyinfo.h"
#include "kmodifierkeyinfoprovider_p.h"
#include <kguiaddons_debug.h>

#include <QGuiApplication>
#include <QPluginLoader>

KModifierKeyInfoProvider *createProvider()
{
    QPluginLoader loader(QStringLiteral("kf" QT_STRINGIFY(QT_VERSION_MAJOR)) + QStringLiteral("/kguiaddons/kmodifierkey/kmodifierkey_")
                         + qGuiApp->platformName());
    auto instance = dynamic_cast<KModifierKeyInfoProvider *>(loader.instance());
    if (instance) {
        return instance;
    }
    qCWarning(KGUIADDONS_LOG) << "Error: could not load plugin for platform" << loader.fileName() << "error:" << loader.errorString() << loader.instance();
    return new KModifierKeyInfoProvider;
}

KModifierKeyInfo::KModifierKeyInfo(QObject *parent)
    : QObject(parent)
    , m_provider(createProvider())
{
    auto *provider = m_provider.get();
    connect(provider, &KModifierKeyInfoProvider::keyPressed, this, &KModifierKeyInfo::keyPressed);
    connect(provider, &KModifierKeyInfoProvider::keyLatched, this, &KModifierKeyInfo::keyLatched);
    connect(provider, &KModifierKeyInfoProvider::keyLocked, this, &KModifierKeyInfo::keyLocked);
    connect(provider, &KModifierKeyInfoProvider::buttonPressed, this, &KModifierKeyInfo::buttonPressed);
    connect(provider, &KModifierKeyInfoProvider::keyAdded, this, &KModifierKeyInfo::keyAdded);
    connect(provider, &KModifierKeyInfoProvider::keyRemoved, this, &KModifierKeyInfo::keyRemoved);
}

KModifierKeyInfo::~KModifierKeyInfo()
{
}

bool KModifierKeyInfo::knowsKey(Qt::Key key) const
{
    return m_provider->knowsKey(key);
}

const QList<Qt::Key> KModifierKeyInfo::knownKeys() const
{
    return m_provider->knownKeys();
}

bool KModifierKeyInfo::isKeyPressed(Qt::Key key) const
{
    return m_provider->isKeyPressed(key);
}

bool KModifierKeyInfo::isKeyLatched(Qt::Key key) const
{
    return m_provider->isKeyLatched(key);
}

bool KModifierKeyInfo::setKeyLatched(Qt::Key key, bool latched)
{
    return m_provider->setKeyLatched(key, latched);
}

bool KModifierKeyInfo::isKeyLocked(Qt::Key key) const
{
    return m_provider->isKeyLocked(key);
}

bool KModifierKeyInfo::setKeyLocked(Qt::Key key, bool locked)
{
    return m_provider->setKeyLocked(key, locked);
}

bool KModifierKeyInfo::isButtonPressed(Qt::MouseButton button) const
{
    return m_provider->isButtonPressed(button);
}
