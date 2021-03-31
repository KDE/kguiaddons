/*
    SPDX-FileCopyrightText: 2003, 2008 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QFontMetrics>

#include <QTest>

#include "kwordwrap.h"
#include <QObject>

class KWordWrap_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        // Qt5 TODO: how to set the dpi?
        // Only found readonly QScreen::logicalDotsPerInch...
#if 0
        QX11Info::setAppDpiX(0, 96);
        QX11Info::setAppDpiY(0, 96);
#endif
    }

    void oldTruncationTest()
    {
        QFont font(QStringLiteral("helvetica"), 12); // let's hope we all have the same...
        QFontMetrics fm(font);
        QRect r(0, 0, 100, -1);
        QString str = QStringLiteral("test wadabada [/foo/bar/waba] and some more text here a");
        KWordWrap ww = KWordWrap::formatText(fm, r, 0, str);
        // qDebug() << str << " => " << ww.truncatedString();
        QCOMPARE(ww.truncatedString(), QStringLiteral("test ..."));

        r.setWidth(200);
        ww = KWordWrap::formatText(fm, r, 0, str);
        QCOMPARE(ww.truncatedString(), QStringLiteral("test wadabada [/foo/bar/..."));

        r.setWidth(300);
        ww = KWordWrap::formatText(fm, r, 0, str);
        QCOMPARE(ww.truncatedString(), QStringLiteral("test wadabada [/foo/bar/waba] and ..."));

        r.setWidth(320);
        ww = KWordWrap::formatText(fm, r, 0, str);
        QCOMPARE(ww.truncatedString(), QStringLiteral("test wadabada [/foo/bar/waba] and ..."));

        r.setWidth(330);
        ww = KWordWrap::formatText(fm, r, 0, str);
        QCOMPARE(ww.truncatedString(), QStringLiteral("test wadabada [/foo/bar/waba] and some ..."));

        r.setWidth(450);
        ww = KWordWrap::formatText(fm, r, 0, str);
        QCOMPARE(ww.truncatedString(), QStringLiteral("test wadabada [/foo/bar/waba] and some more text ..."));

        r.setWidth(460);
        ww = KWordWrap::formatText(fm, r, 0, str);
        QCOMPARE(ww.truncatedString(), QStringLiteral("test wadabada [/foo/bar/waba] and some more text here a"));

        // Reset to the initial rect width
        r.setWidth(100);
        str = QStringLiteral("</p></p></p></p>");
        for (; r.width() > 0; r.setWidth(r.width() - 10)) {
            ww = KWordWrap::formatText(fm, r, 0, str);
            // qDebug() << str << " => " << ww.truncatedString();
            QVERIFY(ww.truncatedString().endsWith("..."));
        }
    }

    void testWithExistingNewlines() // when the input string has \n already
    {
        QRect r(0, 0, 1000, -1); // very wide
        QFont font(QStringLiteral("helvetica"), 12); // let's hope we all have the same...
        QFontMetrics fm(font);
        QString inputString = QStringLiteral("The title here\nFoo (bar)\nFoo2 (bar2)");
        KWordWrap ww = KWordWrap::formatText(fm, r, 0, inputString);
        QString str = ww.wrappedString();
        QCOMPARE(str, inputString);
    }
};

QTEST_MAIN(KWordWrap_UnitTest)

#include "kwordwraptest.moc"
