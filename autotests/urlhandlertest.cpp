/*
    SPDX-FileCopyrightText: 2021 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "urlhandlertest.h"
#include "urlhandler.h"

#include <QDesktopServices>
#include <QTest>

void UrlHandlerTest::testOpeningDocsUrls_data()
{
    QTest::addColumn<QString>("appname");
    QTest::addColumn<QString>("helpUrl");
    QTest::addColumn<QString>("expected");

    QTest::newRow("okular-help-handbook")
        << "okular"
        << "help:/" //
        << QLatin1String("https://docs.kde.org/index.php?branch=stable5&language=%1&application=okular&path=index.html").arg(QLocale().name());

    QTest::newRow("okular-help-config-dialog")
        << "okular"
        << "help:/okular/configure.html"
        << QLatin1String("https://docs.kde.org/index.php?branch=stable5&language=%1&application=okular&path=configure.html").arg(QLocale().name());

    QTest::newRow("systemsettings-fonts")
        << "systemsettings"
        << "help:/kcontrol/fonts/index.html"
        << QLatin1String("https://docs.kde.org/index.php?branch=stable5&language=%1&application=kcontrol/fonts&path=index.html").arg(QLocale().name());

    QTest::newRow("systemsettings-clock")
        << "systemsettings"
        << "help:/kcontrol/clock/index.html"
        << QLatin1String("https://docs.kde.org/index.php?branch=stable5&language=%1&application=kcontrol/clock&path=index.html").arg(QLocale().name());

    QTest::newRow("with-fragment")
        << "kinfocenter"
        << "help:/kinfocenter/index.html#kcm_memory"
        << QLatin1String("https://docs.kde.org/index.php?branch=stable5&language=%1&application=kinfocenter&path=kcm_memory.html").arg(QLocale().name());

    QTest::newRow("with-fragment-2")
        << "okular"
        << "help:/okular/signatures.html#adding_digital_signatures"
        << QLatin1String("https://docs.kde.org/index.php?branch=stable5&language=%1&application=okular&path=signatures.html#adding_digital_signatures")
               .arg(QLocale().name());
}

void UrlHandlerTest::testOpeningDocsUrls()
{
    QFETCH(QString, appname);
    QFETCH(QString, helpUrl);
    QFETCH(QString, expected);

    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationName(appname);

    UrlHandler handler;

    const QUrl actual = handler.concatDocsUrl(QUrl(helpUrl));
    // qDebug() << actual;
    QCOMPARE(actual, QUrl(expected));
}

QTEST_MAIN(UrlHandlerTest)
