/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KINPUTSEQUENCE_H
#define KINPUTSEQUENCE_H

#include <kguiaddons_export.h>

#include <QKeySequence>
#include <QObject>

#include <memory>

/**
 * @brief A input sequence consisting of keystrokes and mouse buttons.
 *
 * Unlike QKeySequence, this class can hold keyboard events and mouse events.
 *
 * @sa QKeySequence, KInputSequenceRecorder
 */
class KGUIADDONS_EXPORT KInputSequence
{
public:
    /**
     * @brief Constructs an empty input sequence.
     */
    KInputSequence();
    explicit KInputSequence(const QString &key, QKeySequence::SequenceFormat format = QKeySequence::SequenceFormat::NativeText);
    explicit KInputSequence(QKeySequence keySequence, Qt::MouseButtons mouseButtons);

    bool isEmpty() const;
    int count() const;

    QKeySequence keySequence() const;
    Qt::MouseButtons mouseButtons() const;

    QString toString(QKeySequence::SequenceFormat format = QKeySequence::SequenceFormat::PortableText) const;

    bool operator==(const KInputSequence &other) const
    {
        return m_keySequence == other.m_keySequence && m_mouseButtons == other.m_mouseButtons;
    }

    bool operator==(const QKeySequence &other) const
    {
        return m_keySequence == other;
    }

    bool operator==(const Qt::MouseButtons &other) const
    {
        return m_mouseButtons == other;
    }

private:
    static QString displayNameForMouseButton(Qt::MouseButton button);

    QKeySequence m_keySequence;
    Qt::MouseButtons m_mouseButtons;
};

#endif
