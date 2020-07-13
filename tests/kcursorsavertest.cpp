/*
    SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "kcursorsaver.h"
#include <thread>
#include <chrono>

class TestWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TestWidget(QWidget *parent = nullptr);
};

TestWidget::TestWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QPushButton *busyButton = new QPushButton(QStringLiteral("busy"), this);
    layout->addWidget(busyButton);
    connect(busyButton, &QPushButton::clicked, this, [this]() {
        KCursorSaver busy(KCursorSaver::busy());
        QMessageBox::information(this, QStringLiteral("info"), QStringLiteral("text"));
    });
}

int main(int argc, char *argv[])
{
    QApplication::setApplicationName(QStringLiteral("testwidget"));

    QApplication app(argc, argv);
    TestWidget mainWidget;
    mainWidget.show();

    return app.exec();
}

#include "kcursorsavertest.moc"
