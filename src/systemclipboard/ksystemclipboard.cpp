/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ksystemclipboard.h"
#include "kguiaddons_debug.h"

#include "qtclipboard_p.h"
#include "waylandclipboard_p.h"
#include "wlrwaylandclipboard_p.h"

#include <QDebug>
#include <QGuiApplication>
#include <QMimeData>
#include <memory>

Q_GLOBAL_STATIC(std::unique_ptr<KSystemClipboard>, s_systemClipboard)

KSystemClipboard *KSystemClipboard::instance()
{
    if (!qGuiApp || qGuiApp->closingDown()) {
        return nullptr;
    }

#ifdef WITH_WAYLAND
    if (!*s_systemClipboard && qGuiApp->platformName() == QLatin1String("wayland")) {
        if (auto waylandClipboard = WaylandClipboard::create(nullptr)) {
            s_systemClipboard->reset(waylandClipboard);
        } else if (auto waylandClipboard = WlrWaylandClipboard::create(nullptr)) {
            s_systemClipboard->reset(waylandClipboard);
        } else {
            qCWarning(KGUIADDONS_LOG) << "Could not init WaylandClipboard, falling back to QtClipboard.";
        }
    }
#endif

    if (!*s_systemClipboard) {
        s_systemClipboard->reset(new QtClipboard(nullptr));
    }

    return s_systemClipboard->get();
}

QString KSystemClipboard::text(QClipboard::Mode mode)
{
    const QMimeData *data = mimeData(mode);
    if (data) {
        return data->text();
    }
    return QString();
}

KSystemClipboard::KSystemClipboard(QObject *parent)
    : QObject(parent)
{
}

#include "moc_ksystemclipboard.cpp"
