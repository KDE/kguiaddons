/*
    SPDX-License-Identifier: LGPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>
*/

#include "kcursorsavertest.h"
#include "util/kcursorsaver.h"
#include <QTest>
QTEST_MAIN(KCursorSaverTest)
static QtMessageHandler s_originalMessageHandler = nullptr;
static void testMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &text)
{
    if (s_originalMessageHandler)
        s_originalMessageHandler(type, context, text);

    if (type == QtWarningMsg && text != QStringLiteral("This KCursorSaver doesn't own the cursor anymore, invalid call to restoreCursor().")) {
        QFAIL("Test caused warning");
    }
}

KCursorSaverTest::KCursorSaverTest(QObject *parent)
    : QObject(parent)
{

}

void KCursorSaverTest::initTestCase()
{
    s_originalMessageHandler = qInstallMessageHandler(testMessageHandler);
}

void KCursorSaverTest::ignoreWarning()
{
    {
        //Emit qwarning when we call twice restoreCursor
        KCursorSaver saver(Qt::WaitCursor);
        saver.restoreCursor();
        saver.restoreCursor();
    }
    {
        //Emit qwarning when "moving A to B and then calling A::restoreCursor()"
        KCursorSaver saverA(Qt::WaitCursor);
        KCursorSaver saverB(std::move(saverA));
        Q_UNUSED(saverB);
        saverA.restoreCursor();
    }
}
