/*
    SPDX-FileCopyrightText: 2021 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef URLHANDLERTEST_H
#define URLHANDLERTEST_H

#include <QObject>

class UrlHandlerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testOpeningDocsUrls_data();
    void testOpeningDocsUrls();
};

#endif // URLHANDLERTEST_H
