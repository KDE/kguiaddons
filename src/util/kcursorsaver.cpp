/*
    SPDX-License-Identifier: LGPL-2.0-or-later
    SPDX-FileCopyrightText: 2003 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>
*/



#include "kcursorsaver.h"

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
    QGuiApplication::restoreOverrideCursor();
    d->inited = false;
}

KCursorSaver KCursorSaver::idle()
{
    return KCursorSaver(Qt::ArrowCursor);
}

KCursorSaver KCursorSaver::busy()
{
    return KCursorSaver(Qt::WaitCursor);
}
