#pragma once

#include <QObject>

#include "kguiaddons_export.h"

class KGUIADDONS_EXPORT KTabletPadController : public QObject
{
    Q_OBJECT
public:
    KTabletPadController(QObject *parent = nullptr);

    struct PadEvent {
        enum State {
            Pressed,
            Released,
        };

        int button;
        State state;
    };

Q_SIGNALS:
    void buttonEvent(const PadEvent &event);

private:
    friend class TabletPad;
    void gotButton(bool pressed, int button);
};
