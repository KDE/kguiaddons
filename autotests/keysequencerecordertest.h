/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
*/

#ifndef KEYSEQUENCERECORDERTEST_H
#define KEYSEQUENCERECORDERTEST_H

#include <QObject>

class QWindow;

class KeySequenceRecorderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testValidWindow();
    void testRecording();
    void testModifiers();
    void testModifierless();
    void testMultiKeyAllowed();

private:
    QWindow *m_window;
};

#endif
