/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kinputsequencetest.h"

#include "kinputsequence.h"

#include <QSignalSpy>
#include <QTest>
#include <QWindow>

QTEST_MAIN(KInputSequenceTest)

void KInputSequenceTest::keyboardOnlyCase()
{
    // empty
    QCOMPARE(KInputSequence(QStringLiteral("")), QKeySequence());

    // modifier + button
    KInputSequence sequence(QStringLiteral("CTRL+O"));
    QCOMPARE(sequence, QKeySequence(QStringLiteral("CTRL+O")));

    // serialized back to string
    QCOMPARE(sequence.toString(), QStringLiteral("Ctrl+O"));
}

void KInputSequenceTest::mouseOnlyCase()
{
    // empty
    QCOMPARE(KInputSequence(QStringLiteral("")), Qt::MouseButtons());

    // left button
    // NOTE: These are intentionally capitalized to test if it can check lowercase + uppercase
    QCOMPARE(KInputSequence(QStringLiteral("LEFTCLICK")), Qt::MouseButtons(Qt::MouseButton::LeftButton));

    // right button
    QCOMPARE(KInputSequence(QStringLiteral("RIGHTCLICK")), Qt::MouseButtons(Qt::MouseButton::RightButton));

    // middle button
    QCOMPARE(KInputSequence(QStringLiteral("MIDDLECLICK")), Qt::MouseButtons(Qt::MouseButton::MiddleButton));

    // putting two of the same button in should do nothing, of course
    QCOMPARE(KInputSequence(QStringLiteral("LEFTCLICK+LeftClick")), Qt::MouseButtons(Qt::MouseButton::LeftButton));

    // two buttons
    QCOMPARE(KInputSequence(QStringLiteral("LEFTCLICK+RIGHTCLICK")), Qt::MouseButtons(Qt::MouseButton::LeftButton | Qt::MouseButton::RightButton));

    // three buttons
    KInputSequence sequence(QStringLiteral("LEFTCLICK+RIGHTCLICK+MIDDLECLICK"));
    QCOMPARE(sequence, Qt::MouseButtons(Qt::MouseButton::LeftButton | Qt::MouseButton::RightButton | Qt::MouseButton::MiddleButton));

    // serialized back to string
    QCOMPARE(sequence.toString(), QStringLiteral("LeftClick+RightClick+MiddleClick"));
}

void KInputSequenceTest::mouseAndKeyboardCase()
{
    // mouse button + keyboard button
    {
        const KInputSequence sequence(QStringLiteral("LEFTCLICK+O"));
        QCOMPARE(sequence.mouseButtons(), Qt::MouseButtons(Qt::MouseButton::LeftButton));
        QCOMPARE(sequence.keySequence(), QKeySequence(QStringLiteral("O")));
    }

    // mouse button + keyboard modifier
    {
        const KInputSequence sequence(QStringLiteral("LEFTCLICK+CTRL"));
        QCOMPARE(sequence.mouseButtons(), Qt::MouseButtons(Qt::MouseButton::LeftButton));
        QCOMPARE(sequence.keySequence(), QKeySequence(QStringLiteral("CTRL")));
    }

    // mouse button + keyboard modifier + keyboard button
    {
        const KInputSequence sequence(QStringLiteral("LEFTCLICK+CTRL+O"));
        QCOMPARE(sequence.mouseButtons(), Qt::MouseButtons(Qt::MouseButton::LeftButton));
        QCOMPARE(sequence.keySequence(), QKeySequence(QStringLiteral("CTRL+O")));

        // serialize!
        QCOMPARE(sequence.toString(), QStringLiteral("Ctrl+O+LeftClick"));
    }
}

#include "moc_kinputsequencetest.cpp"
