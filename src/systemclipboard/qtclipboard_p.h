/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTCLIPBOARD_H
#define QTCLIPBOARD_H

#include "ksystemclipboard.h"

class QtClipboard : public KSystemClipboard
{
    Q_OBJECT
public:
    explicit QtClipboard(QObject *parent);
    void setMimeData(QMimeData *mime, QClipboard::Mode mode) override;
    void clear(QClipboard::Mode mode) override;
    const QMimeData *mimeData(QClipboard::Mode mode) const override;
    bool ownsSelection() const;
    bool ownsClipboard() const;
};

#endif
