/*
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <kguiaddons_debug.h>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QLocale>
#include <QObject>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>

class UrlHandler : public QObject
{
    Q_OBJECT
public:
    explicit UrlHandler(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

public Q_SLOTS:
    void openHelp(const QUrl &url)
    {
        const QString appName = QCoreApplication::applicationName();

        QUrl u(url);
        if (u.path() == QLatin1Char('/')) {
            u.setPath(appName);
        }

        const QString helpcenter = QStandardPaths::findExecutable(QStringLiteral("khelpcenter"));
        if (!helpcenter.isEmpty()) { // use khelpcenter if it is available
            QProcess::startDetached(helpcenter, QStringList(u.toString()));
            return;
        }

        // if khelpcenter is not available and it's a KDE application, use docs.kde.org
        if (QCoreApplication::organizationDomain() == QLatin1String("kde.org")) {
            QString path = url.path();
            QString docPath;
            if (appName == QLatin1String("systemsettings") && path.startsWith(QLatin1String("/kcontrol"))) {
                // special case for kcm modules
                // e.g. "help:/kcontrol/fonts/index.html" >>> "&application=kcontrol/fonts"
                docPath = path.remove(0, 1).remove(QLatin1String("/index.html"));
            } else { // e.g. "help:/okular", "help:/systemsettings"
                docPath = appName + QStringLiteral("&path=") + path;
            }
            QDesktopServices::openUrl(
                QUrl{QLatin1String("https://docs.kde.org/index.php?branch=stable5&language=%1&application=%2").arg(QLocale().name(), docPath)});
            return;
        }

        // not a KDE application
        qCWarning(KGUIADDONS_LOG) << "Could not find a suitable handler for " << u.toString();
    }
};

Q_GLOBAL_STATIC(UrlHandler, s_handler)

static void initializeGlobalSettings()
{
    QDesktopServices::setUrlHandler(QStringLiteral("help"), s_handler, "openHelp");
}

Q_COREAPP_STARTUP_FUNCTION(initializeGlobalSettings)

#include "urlhandler.moc"
