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
    bool ownsCursor = true;
};

KCursorSaver::KCursorSaver(Qt::CursorShape shape)
    : d(std::make_unique<KCursorSaverPrivate>())
{
    QGuiApplication::setOverrideCursor(QCursor(shape));
    d->ownsCursor = true;
}

KCursorSaver::KCursorSaver(KCursorSaver &&other) = default;

KCursorSaver::~KCursorSaver()
{
    if (d && d->ownsCursor) {
        QGuiApplication::restoreOverrideCursor();
    }
}

void KCursorSaver::restoreCursor()
{
    if (!d || !d->ownsCursor) {
        qCWarning(KGUIADDONS_LOG) << "This KCursorSaver doesn't own the cursor anymore, invalid call to restoreCursor().";
        return;
    }
    d->ownsCursor = false;
    QGuiApplication::restoreOverrideCursor();
}

KCursorSaver &KCursorSaver::operator=(KCursorSaver &&other) = default;
