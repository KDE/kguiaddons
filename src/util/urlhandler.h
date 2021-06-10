/*
    SPDX-FileCopyrightText: 2021 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef URLHANDLER_H
#define URLHANDLER_H

#include <kguiaddons_export.h>

#include <QObject>
#include <QUrl>

class KGUIADDONS_EXPORT UrlHandler : public QObject
{
    Q_OBJECT
public:
    explicit UrlHandler(QObject *parent = nullptr);
    ~UrlHandler() = default;

private Q_SLOTS:
    void openHelp(const QUrl &url) const;
    void openManpage(const QUrl &url) const;
    void openInfopage(const QUrl &url) const;

private:
    friend class UrlHandlerTest;
    QUrl concatDocsUrl(const QUrl &url) const;
};

#endif // URLHANDLER_H
