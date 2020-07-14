/*
    SPDX-License-Identifier: LGPL-2.0-or-later
    SPDX-FileCopyrightText: 2003 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>
*/



#include "kcursorsaver.h"

#include <QGuiApplication>
#include <QDebug> //TODO replace by qdebug category

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
    if (!d->inited) {
        qWarning() << "This method can't be calling twice. It's already restored.";
        return;
    }
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
