/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

class TabletEvents : public QObject
{
    Q_OBJECT
public:
    TabletEvents(QObject *parent = nullptr);

Q_SIGNALS:
    void padButtonReceived(const QString &path, uint button, bool pressed);
};
