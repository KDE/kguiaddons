// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later
//
#include "privateplugin.h"
#include "blurhashimageprovider.h"

void PrivatePlugin::initializeEngine(QQmlEngine *engine, const char *)
{
    engine->addImageProvider(QLatin1StringView("blurhash"), new BlurHashImageProvider);
}
