/*
    SPDX-License-Identifier: LGPL-2.0-or-later
    SPDX-FileCopyrightText: 2003 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>
*/



#include "kcursorsaver.h"
#include "kguiaddons_debug.h"
#include <QGuiApplication>

class KCursorSaverPrivate
{
public:
    bool inited = true;
};

KCursorSaver::KCursorSaver(Qt::CursorShape shape)
    : d(new KCursorSaverPrivate)
{
    QGuiApplication::setOverrideCursor(QCursor(shape));
    d->inited = true;
}

KCursorSaver::KCursorSaver(KCursorSaver &&other)
    : d(other.d)
{
    *this = std::move(other);
}

KCursorSaver::~KCursorSaver()
{
    if (d->inited) {
        QGuiApplication::restoreOverrideCursor();
    }
    delete d;
}

void KCursorSaver::restoreCursor()
{
    if (!d->inited) {
        qCWarning(KGUIADDONS_LOG) << "This method can't be called twice. The cursor was already restored.";
        return;
    }
    QGuiApplication::restoreOverrideCursor();
}

KCursorSaver &KCursorSaver::operator =(KCursorSaver &&other)
{
    if (this != &other) {
        d->inited = false;
    }
    return *this;
}
