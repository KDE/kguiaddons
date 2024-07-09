/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KINPUTSEQUENCETEST_H
#define KINPUTSEQUENCETEST_H

#include <QObject>

class QWindow;

class KInputSequenceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void keyboardOnlyCase();
    void mouseOnlyCase();
    void mouseAndKeyboardCase();
};

#endif
