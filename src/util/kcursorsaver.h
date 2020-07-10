/*
    SPDX-License-Identifier: LGPL-2.0-or-later
    SPDX-FileCopyrightText: 2003 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>
*/

#ifndef KCURSORSAVER_H
#define KCURSORSAVER_H
#include <kguiaddons_export.h>

#include <QCursor>

/**
 * @short sets a cursor and makes sure it's restored on destruction
 * Create a KCursorSaver object when you want to set the cursor.
 * As soon as it gets out of scope, it will restore the original
 * cursor.
 * @since 5.73
 */
class KCursorSaverPrivate;
class KGUIADDONS_EXPORT KCursorSaver
{
public:
    /// Creates a KCursorSaver, setting the mouse cursor to @p shape.
    explicit KCursorSaver(Qt::CursorShape shape);

    /// restore the cursor
    ~KCursorSaver();

    /// call this to explitly restore the cursor
    void restoreCursor();

    static KCursorSaver idle();
    static KCursorSaver busy();

private:
    void operator=(const KCursorSaver &rhs);
    KCursorSaverPrivate *const d; ///< @internal
};

#endif
