/*
    SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "keysequencerecorder.h"

#include "keyboardgrabber_p.h"
#include "kguiaddons_debug.h"
#include "shortcutinhibition_p.h"
#include "waylandinhibition_p.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QPointer>
#include <QTimer>
#include <QWindow>

#include <array>

class KeySequenceRecorderPrivate : public QObject
{
    Q_OBJECT
public:
    // Copy of QKeySequencePrivate::MaxKeyCount from private header
    enum { MaxKeyCount = 4 };

    KeySequenceRecorderPrivate(KeySequenceRecorder *qq);

    void controlModifierlessTimeout();
    bool eventFilter(QObject *watched, QEvent *event) override;
    void handleKeyPress(QKeyEvent *event);
    void handleKeyRelease(QKeyEvent *event);
    void finishRecording();

    KeySequenceRecorder *q;
    QKeySequence m_currentKeySequence;
    QPointer<QWindow> m_window;
    bool m_isRecording;
    bool m_multiKeyShortcutsAllowed;
    bool m_modifierlessAllowed;

    Qt::KeyboardModifiers m_currentModifiers;
    QTimer m_modifierlessTimer;
    std::unique_ptr<ShortcutInhibition> m_inhibition;
};

constexpr Qt::KeyboardModifiers modifierMask = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier;

// Copied here from KKeyServer
static bool isShiftAsModifierAllowed(int keyQt)
{
    // remove any modifiers
    keyQt &= ~Qt::KeyboardModifierMask;

    // Shift only works as a modifier with certain keys. It's not possible
    // to enter the SHIFT+5 key sequence for me because this is handled as
    // '%' by qt on my keyboard.
    // The working keys are all hardcoded here :-(
    if (keyQt >= Qt::Key_F1 && keyQt <= Qt::Key_F35) {
        return true;
    }

    if (QChar(keyQt).isLetter()) {
        return true;
    }

    switch (keyQt) {
    case Qt::Key_Return:
    case Qt::Key_Space:
    case Qt::Key_Backspace:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
    case Qt::Key_Escape:
    case Qt::Key_Print:
    case Qt::Key_ScrollLock:
    case Qt::Key_Pause:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Insert:
    case Qt::Key_Delete:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Enter:
    case Qt::Key_SysReq:
    case Qt::Key_CapsLock:
    case Qt::Key_NumLock:
    case Qt::Key_Help:
    case Qt::Key_Back:
    case Qt::Key_Forward:
    case Qt::Key_Stop:
    case Qt::Key_Refresh:
    case Qt::Key_Favorites:
    case Qt::Key_LaunchMedia:
    case Qt::Key_OpenUrl:
    case Qt::Key_HomePage:
    case Qt::Key_Search:
    case Qt::Key_VolumeDown:
    case Qt::Key_VolumeMute:
    case Qt::Key_VolumeUp:
    case Qt::Key_BassBoost:
    case Qt::Key_BassUp:
    case Qt::Key_BassDown:
    case Qt::Key_TrebleUp:
    case Qt::Key_TrebleDown:
    case Qt::Key_MediaPlay:
    case Qt::Key_MediaStop:
    case Qt::Key_MediaPrevious:
    case Qt::Key_MediaNext:
    case Qt::Key_MediaRecord:
    case Qt::Key_MediaPause:
    case Qt::Key_MediaTogglePlayPause:
    case Qt::Key_LaunchMail:
    case Qt::Key_Calculator:
    case Qt::Key_Memo:
    case Qt::Key_ToDoList:
    case Qt::Key_Calendar:
    case Qt::Key_PowerDown:
    case Qt::Key_ContrastAdjust:
    case Qt::Key_Standby:
    case Qt::Key_MonBrightnessUp:
    case Qt::Key_MonBrightnessDown:
    case Qt::Key_KeyboardLightOnOff:
    case Qt::Key_KeyboardBrightnessUp:
    case Qt::Key_KeyboardBrightnessDown:
    case Qt::Key_PowerOff:
    case Qt::Key_WakeUp:
    case Qt::Key_Eject:
    case Qt::Key_ScreenSaver:
    case Qt::Key_WWW:
    case Qt::Key_Sleep:
    case Qt::Key_LightBulb:
    case Qt::Key_Shop:
    case Qt::Key_History:
    case Qt::Key_AddFavorite:
    case Qt::Key_HotLinks:
    case Qt::Key_BrightnessAdjust:
    case Qt::Key_Finance:
    case Qt::Key_Community:
    case Qt::Key_AudioRewind:
    case Qt::Key_BackForward:
    case Qt::Key_ApplicationLeft:
    case Qt::Key_ApplicationRight:
    case Qt::Key_Book:
    case Qt::Key_CD:
    case Qt::Key_Clear:
    case Qt::Key_ClearGrab:
    case Qt::Key_Close:
    case Qt::Key_Copy:
    case Qt::Key_Cut:
    case Qt::Key_Display:
    case Qt::Key_DOS:
    case Qt::Key_Documents:
    case Qt::Key_Excel:
    case Qt::Key_Explorer:
    case Qt::Key_Game:
    case Qt::Key_Go:
    case Qt::Key_iTouch:
    case Qt::Key_LogOff:
    case Qt::Key_Market:
    case Qt::Key_Meeting:
    case Qt::Key_MenuKB:
    case Qt::Key_MenuPB:
    case Qt::Key_MySites:
    case Qt::Key_News:
    case Qt::Key_OfficeHome:
    case Qt::Key_Option:
    case Qt::Key_Paste:
    case Qt::Key_Phone:
    case Qt::Key_Reply:
    case Qt::Key_Reload:
    case Qt::Key_RotateWindows:
    case Qt::Key_RotationPB:
    case Qt::Key_RotationKB:
    case Qt::Key_Save:
    case Qt::Key_Send:
    case Qt::Key_Spell:
    case Qt::Key_SplitScreen:
    case Qt::Key_Support:
    case Qt::Key_TaskPane:
    case Qt::Key_Terminal:
    case Qt::Key_Tools:
    case Qt::Key_Travel:
    case Qt::Key_Video:
    case Qt::Key_Word:
    case Qt::Key_Xfer:
    case Qt::Key_ZoomIn:
    case Qt::Key_ZoomOut:
    case Qt::Key_Away:
    case Qt::Key_Messenger:
    case Qt::Key_WebCam:
    case Qt::Key_MailForward:
    case Qt::Key_Pictures:
    case Qt::Key_Music:
    case Qt::Key_Battery:
    case Qt::Key_Bluetooth:
    case Qt::Key_WLAN:
    case Qt::Key_UWB:
    case Qt::Key_AudioForward:
    case Qt::Key_AudioRepeat:
    case Qt::Key_AudioRandomPlay:
    case Qt::Key_Subtitle:
    case Qt::Key_AudioCycleTrack:
    case Qt::Key_Time:
    case Qt::Key_Select:
    case Qt::Key_View:
    case Qt::Key_TopMenu:
    case Qt::Key_Suspend:
    case Qt::Key_Hibernate:
    case Qt::Key_Launch0:
    case Qt::Key_Launch1:
    case Qt::Key_Launch2:
    case Qt::Key_Launch3:
    case Qt::Key_Launch4:
    case Qt::Key_Launch5:
    case Qt::Key_Launch6:
    case Qt::Key_Launch7:
    case Qt::Key_Launch8:
    case Qt::Key_Launch9:
    case Qt::Key_LaunchA:
    case Qt::Key_LaunchB:
    case Qt::Key_LaunchC:
    case Qt::Key_LaunchD:
    case Qt::Key_LaunchE:
    case Qt::Key_LaunchF:
        return true;

    default:
        return false;
    }
}

static bool isOkWhenModifierless(int key)
{
    // this whole function is a hack, but especially the first line of code
    if (QKeySequence(key).toString().length() == 1) {
        return false;
    }

    switch (key) {
    case Qt::Key_Return:
    case Qt::Key_Space:
    case Qt::Key_Tab:
    case Qt::Key_Backtab: // does this ever happen?
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        return false;
    default:
        return true;
    }
}

static QKeySequence appendToSequence(const QKeySequence &sequence, int key)
{
        qCWarning(KGUIADDONS_LOG) << "Invalid sequence size: " << sequence.count();
    if (sequence.count() >= KeySequenceRecorderPrivate::MaxKeyCount) {
        return sequence;
    }

    std::array<int, KeySequenceRecorderPrivate::MaxKeyCount> keys{sequence[0], sequence[1], sequence[2], sequence[3]};
    keys[sequence.count()] = key;
    return QKeySequence(keys[0], keys[1], keys[2], keys[3]);
}

KeySequenceRecorderPrivate::KeySequenceRecorderPrivate(KeySequenceRecorder *qq)
    : QObject(qq)
    , q(qq)
{
}

void KeySequenceRecorderPrivate::controlModifierlessTimeout()
{
    if (m_currentKeySequence != 0 && !m_currentModifiers) {
        // No modifier key pressed currently. Start the timeout
        m_modifierlessTimer.start(600);
    } else {
        // A modifier is pressed. Stop the timeout
        m_modifierlessTimer.stop();
    }
}

bool KeySequenceRecorderPrivate::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_isRecording) {
        return QObject::eventFilter(watched, event);
    }

    if (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::ContextMenu) {
        event->accept();
        return true;
    }
    if (event->type() == QEvent::KeyRelease) {
        handleKeyRelease(static_cast<QKeyEvent *>(event));
        return true;
    }
    if (event->type() == QEvent::KeyPress) {
        handleKeyPress(static_cast<QKeyEvent *>(event));
        return true;
    }
    return QObject::eventFilter(watched, event);
}

void KeySequenceRecorderPrivate::handleKeyPress(QKeyEvent *event)
{
    m_currentModifiers = event->modifiers() & modifierMask;
    int key = event->key();
    switch (key) {
    case -1:
        qCWarning(KGUIADDONS_LOG) << "Got unknown key";
        // Old behavior was to stop recording here instead of continuing like this
        return;
    case 0:
        break;
    case Qt::Key_AltGr:
        // or else we get unicode salad
        break;
    case Qt::Key_Super_L:
    case Qt::Key_Super_R:
        // Qt doesn't properly recognize Super_L/Super_R as MetaModifier
        m_currentModifiers |= Qt::MetaModifier;
        Q_FALLTHROUGH();
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
        controlModifierlessTimeout();
        Q_EMIT q->currentKeySequenceChanged();
        break;
    default:
        if (m_currentKeySequence.count() == 0 && !(m_currentModifiers & ~Qt::ShiftModifier)) {
            // It's the first key and no modifier pressed. Check if this is allowed
            if (!(isOkWhenModifierless(key) || m_modifierlessAllowed)) {
                // No it's not
                return;
            }
        }

        // We now have a valid key press.
        if ((key == Qt::Key_Backtab) && (m_currentModifiers & Qt::ShiftModifier)) {
            key = Qt::Key_Tab | m_currentModifiers;
        } else if (isShiftAsModifierAllowed(key)) {
            key |= m_currentModifiers;
        } else {
            key |= (m_currentModifiers & ~Qt::ShiftModifier);
        }

        m_currentKeySequence = appendToSequence(m_currentKeySequence, key);
        Q_EMIT q->currentKeySequenceChanged();

        if ((!m_multiKeyShortcutsAllowed) || (m_currentKeySequence.count() == MaxKeyCount)) {
            finishRecording();
            break;
        }
        controlModifierlessTimeout();
    }
    event->accept();
}

void KeySequenceRecorderPrivate::handleKeyRelease(QKeyEvent *event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers() & modifierMask;
    switch (event->key()) {
    case -1:
        return;
    case Qt::Key_Super_L:
    case Qt::Key_Super_R:
        // Qt doesn't properly recognize Super_L/Super_R as MetaModifier
        modifiers &= ~Qt::MetaModifier;
    }
    if ((modifiers & m_currentModifiers) < m_currentModifiers) {
        m_currentModifiers = modifiers;
        controlModifierlessTimeout();
        Q_EMIT q->currentKeySequenceChanged();
    }
}

void KeySequenceRecorderPrivate::finishRecording()
{
    m_modifierlessTimer.stop();
    m_isRecording = false;
    m_currentModifiers = Qt::NoModifier;
    if (m_inhibition) {
        m_inhibition->disableInhibition();
    }
    Q_EMIT q->recordingChanged();
    Q_EMIT q->gotKeySequence(m_currentKeySequence);
}

KeySequenceRecorder::KeySequenceRecorder(QWindow *window, QObject *parent)
    : QObject(parent)
    , d(new KeySequenceRecorderPrivate(this))
{
    d->m_isRecording = false;
    d->m_modifierlessAllowed = false;
    d->m_multiKeyShortcutsAllowed = true;

    setWindow(window);
    connect(&d->m_modifierlessTimer, &QTimer::timeout, d.get(), &KeySequenceRecorderPrivate::finishRecording);
}

KeySequenceRecorder::~KeySequenceRecorder() noexcept
{
}

void KeySequenceRecorder::startRecording()
{
    if (!d->m_window) {
        qCWarning(KGUIADDONS_LOG) << "Cannot record without a window";
        return;
    }
    d->m_isRecording = true;
    d->m_currentKeySequence = QKeySequence();
    if (d->m_inhibition) {
        d->m_inhibition->enableInhibition();
    }
    Q_EMIT recordingChanged();
    Q_EMIT currentKeySequenceChanged();
}

bool KeySequenceRecorder::isRecording() const
{
    return d->m_isRecording;
}

QKeySequence KeySequenceRecorder::currentKeySequence() const
{
    return d->m_isRecording ? appendToSequence(d->m_currentKeySequence, d->m_currentModifiers) : d->m_currentKeySequence;
}

QWindow *KeySequenceRecorder::window() const
{
    return d->m_window;
}

void KeySequenceRecorder::setWindow(QWindow *window)
{
    if (window == d->m_window) {
        return;
    }

    if (d->m_window) {
        d->m_window->removeEventFilter(d.get());
    }

    if (window) {
        window->installEventFilter(d.get());
        qCDebug(KGUIADDONS_LOG) << "listening for events in" << window;
    }

    if (qGuiApp->platformName() == QLatin1String("wayland")) {
#ifdef WITH_WAYLAND
        d->m_inhibition.reset(new WaylandInhibition(window));
#endif
    } else {
        d->m_inhibition.reset(new KeyboardGrabber(window));
    }

    d->m_window = window;

    Q_EMIT windowChanged();
}

bool KeySequenceRecorder::multiKeyShortcutsAllowed() const
{
    return d->m_multiKeyShortcutsAllowed;
}

void KeySequenceRecorder::setMultiKeyShortcutsAllowed(bool allowed)
{
    if (allowed == d->m_multiKeyShortcutsAllowed) {
        return;
    }
    d->m_multiKeyShortcutsAllowed = allowed;
    Q_EMIT multiKeyShortcutsAllowedChanged();
}

bool KeySequenceRecorder::modifierlessAllowed() const
{
    return d->m_modifierlessAllowed;
}

void KeySequenceRecorder::setModifierlessAllowed(bool allowed)
{
    if (allowed == d->m_modifierlessAllowed) {
        return;
    }
    d->m_modifierlessAllowed = allowed;
    Q_EMIT modifierlessAllowedChanged();
}

#include "keysequencerecorder.moc"
