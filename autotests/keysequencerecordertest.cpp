/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
*/

#include "keysequencerecordertest.h"

#include "keysequencerecorder.h"

#include <QSignalSpy>
#include <QTest>
#include <QWindow>

QTEST_MAIN(KeySequenceRecorderTest)

void KeySequenceRecorderTest::initTestCase()
{
    m_window = new QWindow;
}

void KeySequenceRecorderTest::cleanupTestCase()
{
    delete m_window;
}

void KeySequenceRecorderTest::testValidWindow()
{
    KeySequenceRecorder recorder(nullptr);
    QSignalSpy recordingSpy(&recorder, &KeySequenceRecorder::recordingChanged);

    recorder.startRecording();
    QCOMPARE(recordingSpy.count(), 0);
    QVERIFY(!recorder.isRecording());

    recorder.setWindow(m_window);
    recorder.startRecording();
    QCOMPARE(recordingSpy.count(), 2);
    QVERIFY(recorder.isRecording());
}

void KeySequenceRecorderTest::testRecording()
{
    KeySequenceRecorder recorder(m_window);
    QSignalSpy recordingSpy(&recorder, &KeySequenceRecorder::recordingChanged);
    QSignalSpy sequenceSpy(&recorder, &KeySequenceRecorder::currentKeySequenceChanged);
    QSignalSpy resultSpy(&recorder, &KeySequenceRecorder::gotKeySequence);

    recorder.startRecording();
    QVERIFY(recorder.isRecording());
    QCOMPARE(recordingSpy.count(), 1);
    QCOMPARE(sequenceSpy.count(), 1);

    QTest::keyClick(m_window, Qt::Key_A, Qt::ControlModifier);
    QVERIFY(recorder.isRecording());
    QCOMPARE(recordingSpy.count(), 1);
    QCOMPARE(sequenceSpy.count(), 4); // two key events + modifier release
    QCOMPARE(resultSpy.count(), 0);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::Key_A | Qt::ControlModifier));

    QTest::qWait(800);
    QCOMPARE(sequenceSpy.count(), 4);
    QCOMPARE(recordingSpy.count(), 2);
    QVERIFY(!recorder.isRecording());
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::Key_A | Qt::ControlModifier));
    QCOMPARE(resultSpy.takeFirst().at(0).value<QKeySequence>(), QKeySequence(Qt::Key_A | Qt::ControlModifier));
}

void KeySequenceRecorderTest::testModifiers()
{
    KeySequenceRecorder recorder(m_window);
    QSignalSpy recordingSpy(&recorder, &KeySequenceRecorder::recordingChanged);
    QSignalSpy sequenceSpy(&recorder, &KeySequenceRecorder::currentKeySequenceChanged);
    QSignalSpy resultSpy(&recorder, &KeySequenceRecorder::gotKeySequence);

    recorder.startRecording();
    QCOMPARE(sequenceSpy.count(), 1);
    QCOMPARE(recordingSpy.count(), 1);
    QVERIFY(recorder.isRecording());

    QTest::keyPress(m_window, Qt::Key_Control);
    QCOMPARE(sequenceSpy.count(), 2);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::ControlModifier));

    // Releasing a different key because when releasing Key_Meta, QTest sends a KeyRelease event
    // with MetaModifier even though modifieres should be empty
    QTest::keyRelease(m_window, Qt::Key_A, Qt::NoModifier);
    QCOMPARE(sequenceSpy.count(), 3);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence());
    QCOMPARE(recordingSpy.count(), 1);
    QVERIFY(recorder.isRecording());

    QTest::qWait(800);
    QVERIFY(recorder.isRecording());

    QTest::keyPress(m_window, Qt::Key_Control);
    QCOMPARE(sequenceSpy.count(), 4);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::ControlModifier));

    QTest::keyPress(m_window, Qt::Key_Alt, Qt::ControlModifier);
    QCOMPARE(sequenceSpy.count(), 6); // QTest sends two key events, one for each modifier
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::AltModifier | Qt::ControlModifier));

    QCOMPARE(resultSpy.count(), 0); // modifierless not allowed
}

void KeySequenceRecorderTest::testModifierless()
{
    KeySequenceRecorder recorder(m_window);
    QSignalSpy resultSpy(&recorder, &KeySequenceRecorder::gotKeySequence);
    QSignalSpy sequenceSpy(&recorder, &KeySequenceRecorder::currentKeySequenceChanged);

    recorder.startRecording();
    QVERIFY(recorder.isRecording());
    QCOMPARE(sequenceSpy.count(), 1);

    recorder.setModifierlessAllowed(false);
    QTest::keyPress(m_window, Qt::Key_A);
    QTest::qWait(800);
    QVERIFY(recorder.isRecording());
    QCOMPARE(sequenceSpy.count(), 1);
    QCOMPARE(resultSpy.count(), 0);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence());

    recorder.setModifierlessAllowed(true);
    QTest::keyPress(m_window, Qt::Key_A);
    QTest::qWait(800);
    QVERIFY(!recorder.isRecording());
    QCOMPARE(sequenceSpy.count(), 2);
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::Key_A));
}

void KeySequenceRecorderTest::testMultiKeyAllowed()
{
    KeySequenceRecorder recorder(m_window);
    QSignalSpy recordingSpy(&recorder, &KeySequenceRecorder::recordingChanged);
    QSignalSpy resultSpy(&recorder, &KeySequenceRecorder::gotKeySequence);

    recorder.startRecording();

    recorder.setMultiKeyShortcutsAllowed(true);
    int keys[4] = {0};
    for (int i = 0; i < 4; ++i) {
        QVERIFY(recorder.isRecording());
        QCOMPARE(recordingSpy.count(), 1);
        QCOMPARE(resultSpy.count(), 0);
        keys[i] = Qt::Key_A | Qt::ControlModifier;
        QKeySequence result(keys[0], keys[1], keys[2], keys[3]);
        QTest::keyPress(m_window, Qt::Key_A, Qt::ControlModifier);
        QTest::keyRelease(m_window, Qt::Key_A, Qt::ControlModifier);
        QCOMPARE(recorder.currentKeySequence(), QKeySequence(keys[0], keys[1], keys[2], keys[3]));
    }
    QVERIFY(!recorder.isRecording());
    QCOMPARE(recordingSpy.count(), 2);
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(resultSpy.takeFirst().at(0).value<QKeySequence>(), QKeySequence(keys[0], keys[1], keys[2], keys[3]));

    recorder.setMultiKeyShortcutsAllowed(false);
    recorder.startRecording();
    QVERIFY(recorder.isRecording());
    QCOMPARE(recordingSpy.count(), 4);
    QTest::keyPress(m_window, Qt::Key_A, Qt::ControlModifier);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::Key_A | Qt::ControlModifier));
    QVERIFY(!recorder.isRecording());
    QCOMPARE(recordingSpy.count(), 5);
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(resultSpy.takeAt(0).at(0).value<QKeySequence>(), QKeySequence(Qt::Key_A | Qt::ControlModifier));
}
