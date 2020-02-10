/*
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QObject>
#include <QUrl>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QProcess>
#include <QDesktopServices>
#include <QLocale>

class UrlHandler : public QObject
{
    Q_OBJECT
public:
    explicit UrlHandler(QObject *parent = nullptr) : QObject(parent) {}

public Q_SLOTS:
    void openHelp(const QUrl &url)
    {
        QUrl u(url);
        if (u.path() == QLatin1Char('/')) {
            u.setPath(QCoreApplication::applicationName());
        }

        const QString helpcenter = QStandardPaths::findExecutable(QStringLiteral("khelpcenter"));
        if (helpcenter.isEmpty()) {
            if (QCoreApplication::organizationDomain() == QLatin1String("kde.org")) {
                //if khelpcenter is not installed and it's a KDE application, use docs.kde.org
                const QUrl httpUrl(QLatin1String("https://docs.kde.org/index.php?branch=stable5&language=")+QLocale().name()+QLatin1String("&application=") +
                    QCoreApplication::applicationName() + QStringLiteral("&path=") + url.path());
                QDesktopServices::openUrl(httpUrl);
            } else
                QDesktopServices::openUrl(u);
        } else {
            QProcess::startDetached(helpcenter, QStringList(u.toString()));
        }
    }
};

Q_GLOBAL_STATIC(UrlHandler, s_handler)

static void initializeGlobalSettings()
{
    QDesktopServices::setUrlHandler(QStringLiteral("help"), s_handler, "openHelp");
}

Q_COREAPP_STARTUP_FUNCTION(initializeGlobalSettings)

#include "urlhandler.moc"
