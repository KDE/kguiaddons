/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kinputsequence.h"

// TODO: Expand to other buttons in the future
const QList consideredButtons = {Qt::LeftButton, Qt::RightButton, Qt::MiddleButton};

KInputSequence::KInputSequence() = default;

KInputSequence::KInputSequence(const QString &key, const QKeySequence::SequenceFormat format)
{
    QStringList inputs = key.split(QLatin1Char('+'));

    // First we want to check if the sequence we were provided contains mouse buttons.
    // We need to process those seperately and then give the rest to QKeySequence.
    for (const auto consideredButton : consideredButtons) {
        const auto leftButtonName = displayNameForMouseButton(consideredButton);
        if (inputs.contains(leftButtonName, Qt::CaseInsensitive)) {
            inputs.removeAll(leftButtonName); // lowercase
            inputs.removeAll(leftButtonName.toUpper()); // uppercase
            m_mouseButtons |= consideredButton;
        }
    }

    if (!inputs.empty()) {
        m_keySequence = QKeySequence(inputs.join(QLatin1Char('+')), format);
    }
}

KInputSequence::KInputSequence(const QKeySequence keySequence, Qt::MouseButtons mouseButtons)
{
    m_keySequence = keySequence;
    m_mouseButtons = mouseButtons;
}

bool KInputSequence::isEmpty() const
{
    return count() == 0;
}

int KInputSequence::count() const
{
    int count = m_keySequence.count();
    for (auto consideredButton : consideredButtons) {
        if (m_mouseButtons.testFlag(consideredButton)) {
            ++count;
        }
    }

    return count;
}

QKeySequence KInputSequence::keySequence() const
{
    return m_keySequence;
}

Qt::MouseButtons KInputSequence::mouseButtons() const
{
    return m_mouseButtons;
}

QString KInputSequence::toString(QKeySequence::SequenceFormat format) const
{
    QStringList parts;

    // Add keyboard strings first
    if (!m_keySequence.isEmpty()) {
        parts.append(m_keySequence.toString(format));
    }

    // Then add mouse buttons
    for (auto consideredButton : consideredButtons) {
        if (m_mouseButtons.testFlag(consideredButton)) {
            parts.append(displayNameForMouseButton(consideredButton));
        }
    }

    return parts.join(QLatin1Char('+'));
}

QString KInputSequence::displayNameForMouseButton(const Qt::MouseButton button)
{
    switch (button) {
    case Qt::LeftButton:
        return QStringLiteral("LeftClick");
    case Qt::RightButton:
        return QStringLiteral("RightClick");
    case Qt::MiddleButton:
        return QStringLiteral("MiddleClick");
    default:
        Q_UNREACHABLE();
    }
}
