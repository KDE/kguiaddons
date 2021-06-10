/*
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "urlhandler.h"
#include <kguiaddons_debug.h>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QLocale>
#include <QObject>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>

static bool openHelpWithKHelpCenter(const QUrl &url)
{
    const QString helpcenter = QStandardPaths::findExecutable(QStringLiteral("khelpcenter"));
    if (!helpcenter.isEmpty()) {
        QUrl u(url);
        if (u.path() == QLatin1Char('/')) {
            const QString appName = QCoreApplication::applicationName();
            u.setPath(appName);
        }

        QProcess::startDetached(helpcenter, QStringList(u.toString()));
        return true;
    }

    return false;
}

UrlHandler::UrlHandler(QObject *parent)
    : QObject(parent)
{
}

void UrlHandler::openHelp(const QUrl &url) const
{
    if (openHelpWithKHelpCenter(url)) {
        return;
    }

    const QUrl docUrl = concatDocsUrl(url);
    if (docUrl.isValid()) {
        QDesktopServices::openUrl(docUrl);
    } else {
        qCWarning(KGUIADDONS_LOG) << "Could not find a suitable handler for " << url.toString();
    }
}

QUrl UrlHandler::concatDocsUrl(const QUrl &url) const
{
    if (QCoreApplication::organizationDomain() != QLatin1String("kde.org")) {
        return {};
    }

    // KHelpCenter is not available and it's a KDE application, open the docs at docs.kde.org
    // with the default web browser on the system

    QString path = url.path();
    const QLatin1String common("https://docs.kde.org/index.php?branch=stable5");

    const QString appName = QCoreApplication::applicationName();

    // Special case for KCModules
    if (appName == QLatin1String("systemsettings") && path.startsWith(QLatin1String("/kcontrol"))) {
        // E.g. change "/kcontrol/fonts/index.html" to "&application=kcontrol/fonts&path=index.html"
        // docs.kde.org will resolve the url and add the proper package name, e.g. plasma-workspace:
        // https://docs.kde.org/stable5/en/plasma-workspace/kcontrol/fonts/index.html
        QString kcmAppName(path);
        kcmAppName.remove(0, 1); // Remove leading "/"
        const int idx = kcmAppName.indexOf(QLatin1String("/index.html"));
        if (idx > 0) {
            kcmAppName.truncate(idx);
        }

        const auto docUrl = QUrl(common + QLatin1String("&language=%1&application=%2&path=%3").arg(QLocale().name(), kcmAppName, QLatin1String("index.html")));
        return docUrl;
    }

    // E.g. "help:/" and appName is "okular", e.g. opening Help -> Okular HandBook
    if (path == QLatin1Char('/')) {
        const auto docUrl = QUrl(common + QLatin1String("&language=%1&application=%2&path=%3").arg(QLocale().name(), appName, QLatin1String("index.html")));
        return docUrl;
    }

    // E.g. "help:/okular/configure.html", don't repeat "appName"; e.g. clicking Help button in
    // the "Settings -> Configure Okular" dialog
    if (path.startsWith(QLatin1String("/%1/").arg(appName))) {
        path.remove(0, appName.size() + 2);

        const QString fragment = url.fragment();
        if (!fragment.isEmpty()) {
            // E.g. "help:/kinfocenter/index.html#kcm_memory", it's actually "kinfocenter/kcm_memory.html"
            if (path == QLatin1String("index.html")) {
                path = fragment + QLatin1String(".html");
            } else {
                // E.g. "help:/okular/signatures.html#adding_digital_signatures"
                path += QLatin1Char('#') + fragment;
            }
        }

        const auto docUrl = QUrl(common + QLatin1String("&language=%1&application=%2&path=%3").arg(QLocale().name(), appName, path));
        return docUrl;
    }

    return {};
}

void UrlHandler::openManpage(const QUrl &url) const
{
    QString exec = QStandardPaths::findExecutable(QStringLiteral("khelpcenter"));
    if (exec.isEmpty()) {
        exec = QStandardPaths::findExecutable(QStringLiteral("konqueror"));
    }

    if (!exec.isEmpty()) {
        QProcess::startDetached(exec, QStringList(url.toString()));
    }
}

void UrlHandler::openInfopage(const QUrl &url) const
{
    QString exec = QStandardPaths::findExecutable(QStringLiteral("khelpcenter"));
    if (exec.isEmpty()) {
        exec = QStandardPaths::findExecutable(QStringLiteral("konqueror"));
    }

    if (!exec.isEmpty()) {
        QProcess::startDetached(exec, QStringList(url.toString()));
    }
}

Q_GLOBAL_STATIC(UrlHandler, s_handler)

static void initializeGlobalSettings()
{
    QDesktopServices::setUrlHandler(QStringLiteral("help"), s_handler, "openHelp");
    QDesktopServices::setUrlHandler(QStringLiteral("man"), s_handler, "openManpage");
    QDesktopServices::setUrlHandler(QStringLiteral("info"), s_handler, "openInfopage");
}

Q_COREAPP_STARTUP_FUNCTION(initializeGlobalSettings)
