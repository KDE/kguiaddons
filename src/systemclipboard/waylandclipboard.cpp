/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "waylandclipboard_p.h"

#include <QBuffer>
#include <QFile>
#include <QGuiApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QMimeData>
#include <QPointer>
#include <QWaylandClientExtension>
#include <QWindow>
#include <QtWaylandClientVersion>
#include <qpa/qplatformnativeinterface.h>

#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "qwayland-wayland.h"
#include "qwayland-wlr-data-control-unstable-v1.h"

static inline QString applicationQtXImageLiteral()
{
    return QStringLiteral("application/x-qt-image");
}

// copied from https://code.woboq.org/qt5/qtbase/src/gui/kernel/qinternalmimedata.cpp.html
static QString utf8Text()
{
    return QStringLiteral("text/plain;charset=utf-8");
}

static QStringList imageMimeFormats(const QList<QByteArray> &imageFormats)
{
    QStringList formats;
    formats.reserve(imageFormats.size());
    for (const auto &format : imageFormats)
        formats.append(QLatin1String("image/") + QLatin1String(format.toLower()));
    // put png at the front because it is best
    int pngIndex = formats.indexOf(QLatin1String("image/png"));
    if (pngIndex != -1 && pngIndex != 0)
        formats.move(pngIndex, 0);
    return formats;
}

static inline QStringList imageReadMimeFormats()
{
    return imageMimeFormats(QImageReader::supportedImageFormats());
}

static inline QStringList imageWriteMimeFormats()
{
    return imageMimeFormats(QImageWriter::supportedImageFormats());
}
// end copied

class DataControlDeviceManager : public QWaylandClientExtensionTemplate<DataControlDeviceManager>, public QtWayland::zwlr_data_control_manager_v1
{
    Q_OBJECT
public:
    DataControlDeviceManager()
        : QWaylandClientExtensionTemplate<DataControlDeviceManager>(2)
    {
    }

    void instantiate()
    {
        initialize();
    }

    ~DataControlDeviceManager()
    {
        if (isInitialized()) {
            destroy();
        }
    }
};

class DataControlOffer : public QMimeData, public QtWayland::zwlr_data_control_offer_v1
{
    Q_OBJECT
public:
    DataControlOffer(struct ::zwlr_data_control_offer_v1 *id)
        : QtWayland::zwlr_data_control_offer_v1(id)
    {
    }

    ~DataControlOffer()
    {
        destroy();
    }

    QStringList formats() const override
    {
        return m_receivedFormats;
    }

    bool containsImageData() const
    {
        if (m_receivedFormats.contains(applicationQtXImageLiteral())) {
            return true;
        }
        const auto formats = imageReadMimeFormats();
        for (const auto &receivedFormat : m_receivedFormats) {
            if (formats.contains(receivedFormat)) {
                return true;
            }
        }
        return false;
    }

    bool hasFormat(const QString &mimeType) const override
    {
        if (mimeType == QStringLiteral("text/plain") && m_receivedFormats.contains(utf8Text())) {
            return true;
        }
        if (m_receivedFormats.contains(mimeType)) {
            return true;
        }

        // If we have image data
        if (containsImageData()) {
            // is the requested output mimeType supported ?
            const QStringList imageFormats = imageWriteMimeFormats();
            for (const QString &imageFormat : imageFormats) {
                if (imageFormat == mimeType) {
                    return true;
                }
            }
            if (mimeType == applicationQtXImageLiteral()) {
                return true;
            }
        }

        return false;
    }

protected:
    void zwlr_data_control_offer_v1_offer(const QString &mime_type) override
    {
        m_receivedFormats << mime_type;
    }

    QVariant retrieveData(const QString &mimeType, QMetaType type) const override;

private:
    /** reads data from a file descriptor with a timeout of 1 second
     *  true if data is read successfully
     */
    static bool readData(int fd, QByteArray &data);
    QStringList m_receivedFormats;
};

QVariant DataControlOffer::retrieveData(const QString &mimeType, QMetaType type) const
{
    Q_UNUSED(type);

    QString mime;
    if (!m_receivedFormats.contains(mimeType)) {
        if (mimeType == QStringLiteral("text/plain") && m_receivedFormats.contains(utf8Text())) {
            mime = utf8Text();
        } else if (mimeType == applicationQtXImageLiteral()) {
            const auto writeFormats = imageWriteMimeFormats();
            for (const auto &receivedFormat : m_receivedFormats) {
                if (writeFormats.contains(receivedFormat)) {
                    mime = receivedFormat;
                    break;
                }
            }
            if (mime.isEmpty()) {
                // default exchange format
                mime = QStringLiteral("image/png");
            }
        }

        if (mime.isEmpty()) {
            return QVariant();
        }
    } else {
        mime = mimeType;
    }

    int pipeFds[2];
    if (pipe(pipeFds) != 0) {
        return QVariant();
    }

    auto t = const_cast<DataControlOffer *>(this);
    t->receive(mime, pipeFds[1]);

    close(pipeFds[1]);

    /*
     * Ideally we need to introduce a non-blocking QMimeData object
     * Or a non-blocking constructor to QMimeData with the mimetypes that are relevant
     *
     * However this isn't actually any worse than X.
     */

    QPlatformNativeInterface *native = qGuiApp->platformNativeInterface();
    auto display = static_cast<struct ::wl_display *>(native->nativeResourceForIntegration("wl_display"));
    wl_display_flush(display);

    QFile readPipe;
    if (readPipe.open(pipeFds[0], QIODevice::ReadOnly)) {
        QByteArray data;
        if (readData(pipeFds[0], data)) {
            close(pipeFds[0]);

            if (mimeType == applicationQtXImageLiteral()) {
                QImage img = QImage::fromData(data, mime.mid(mime.indexOf(QLatin1Char('/')) + 1).toLatin1().toUpper().data());
                if (!img.isNull()) {
                    return img;
                }
            }
            return data;
        }
        close(pipeFds[0]);
    }
    return QVariant();
}

bool DataControlOffer::readData(int fd, QByteArray &data)
{
    pollfd pfds[1];
    pfds[0].fd = fd;
    pfds[0].events = POLLIN;

    while (true) {
        const int ready = poll(pfds, 1, 1000);
        if (ready < 0) {
            if (errno != EINTR) {
                qWarning("DataControlOffer: poll() failed: %s", strerror(errno));
                return false;
            }
        } else if (ready == 0) {
            qWarning("DataControlOffer: timeout reading from pipe");
            return false;
        } else {
            char buf[4096];
            int n = read(fd, buf, sizeof buf);

            if (n < 0) {
                qWarning("DataControlOffer: read() failed: %s", strerror(errno));
                return false;
            } else if (n == 0) {
                return true;
            } else if (n > 0) {
                data.append(buf, n);
            }
        }
    }
};

class DataControlDevice : public QObject, public QtWayland::zwlr_data_control_device_v1
{
    Q_OBJECT
public:
    DataControlDevice(struct ::zwlr_data_control_device_v1 *id)
        : QtWayland::zwlr_data_control_device_v1(id)
    {
    }

    ~DataControlDevice()
    {
        destroy();
    }

    QMimeData *receivedSelection()
    {
        return m_receivedSelection.get();
    }

    QMimeData *receivedPrimarySelection()
    {
        return m_receivedPrimarySelection.get();
    }
Q_SIGNALS:
    void receivedSelectionChanged();
    void selectionChanged();

    void receivedPrimarySelectionChanged();
    void primarySelectionChanged();

protected:
    void zwlr_data_control_device_v1_data_offer(struct ::zwlr_data_control_offer_v1 *id) override
    {
        // this will become memory managed when we retrieve the selection event
        // a compositor calling data_offer without doing that would be a bug
        new DataControlOffer(id);
    }

    void zwlr_data_control_device_v1_selection(struct ::zwlr_data_control_offer_v1 *id) override
    {
        if (!id) {
            m_receivedSelection.reset();
        } else {
            auto derivated = QtWayland::zwlr_data_control_offer_v1::fromObject(id);
            auto offer = dynamic_cast<DataControlOffer *>(derivated); // dynamic because of the dual inheritance
            m_receivedSelection.reset(offer);
        }
        Q_EMIT receivedSelectionChanged();
    }

    void zwlr_data_control_device_v1_primary_selection(struct ::zwlr_data_control_offer_v1 *id) override
    {
        if (!id) {
            m_receivedPrimarySelection.reset();
        } else {
            auto derivated = QtWayland::zwlr_data_control_offer_v1::fromObject(id);
            auto offer = dynamic_cast<DataControlOffer *>(derivated); // dynamic because of the dual inheritance
            m_receivedPrimarySelection.reset(offer);
        }
        Q_EMIT receivedPrimarySelectionChanged();
    }

private:
    std::unique_ptr<DataControlOffer> m_receivedSelection; // latest selection set from externally to here
    std::unique_ptr<DataControlOffer> m_receivedPrimarySelection; // latest selection set from externally to here
    friend WaylandClipboard;
};

WaylandClipboard::WaylandClipboard(QObject *parent)
    : KSystemClipboard(parent)
    , m_manager(new DataControlDeviceManager)
{
    connect(m_manager.get(), &DataControlDeviceManager::activeChanged, this, [this]() {
        if (m_manager->isActive()) {
            QPlatformNativeInterface *native = qApp->platformNativeInterface();
            if (!native) {
                return;
            }
            auto seat = static_cast<struct ::wl_seat *>(native->nativeResourceForIntegration("wl_seat"));
            if (!seat) {
                return;
            }
            m_device.reset(new DataControlDevice(m_manager->get_data_device(seat)));

            connect(m_device.get(), &DataControlDevice::receivedSelectionChanged, this, [this]() {
                // // When our source is still valid, so the offer is for setting it or we emit changed when it is cancelled
                // if (!m_device->selection()) {
                // }
                Q_EMIT changed(QClipboard::Clipboard);
            });

            connect(m_device.get(), &DataControlDevice::receivedPrimarySelectionChanged, this, [this]() {
                // When our source is still valid, so the offer is for setting it or we emit changed when it is cancelled
                // if (!m_device->primarySelection()) {
                // }
                Q_EMIT changed(QClipboard::Selection);
            });

        } else {
            m_device.reset();
        }
    });

    connect(QGuiApplication::clipboard(), &QClipboard::changed, this, [this](QClipboard::Mode mode) {
        if (mode == QClipboard::Clipboard) {
            if (QGuiApplication::clipboard()->ownsClipboard()) {
                Q_EMIT changed(QClipboard::Clipboard);
            }
        }
        if (mode == QClipboard::Selection) {
            if (QGuiApplication::clipboard()->ownsSelection()) {
                Q_EMIT changed(QClipboard::Selection);
            }
        }
    });

    m_manager->instantiate();
}

WaylandClipboard::~WaylandClipboard() = default;

bool WaylandClipboard::isValid()
{
    return m_manager && m_manager->isInitialized();
}

void WaylandClipboard::setMimeData(QMimeData *mime, QClipboard::Mode mode)
{
    QGuiApplication::clipboard()->setMimeData(mime, mode);
}

void WaylandClipboard::clear(QClipboard::Mode mode)
{
    if (!m_device) {
        return;
    }
    if (QGuiApplication::clipboard()->ownsClipboard() && mode == QClipboard::Clipboard) {
        QGuiApplication::clipboard()->setMimeData(nullptr, mode);
    }
    if (QGuiApplication::clipboard()->ownsSelection() && mode == QClipboard::Selection) {
        QGuiApplication::clipboard()->setMimeData(nullptr, mode);
    }
}

const QMimeData *WaylandClipboard::mimeData(QClipboard::Mode mode) const
{
    if (!m_device) {
        return nullptr;
    }

    // return our locally set selection if it's not cancelled to avoid copying data to ourselves
    if (mode == QClipboard::Clipboard) {
        // This application owns the clipboard via the regular data_device, use it so we don't block ourselves
        if (QGuiApplication::clipboard()->ownsClipboard()) {
            return QGuiApplication::clipboard()->mimeData(mode);
        }
        return m_device->receivedSelection();
    } else if (mode == QClipboard::Selection) {
        // This application owns the primary selection via the regular primary_selection_device, use it so we don't block ourselves
        if (QGuiApplication::clipboard()->ownsSelection()) {
            return QGuiApplication::clipboard()->mimeData(mode);
        }
        return m_device->receivedPrimarySelection();
    }
    return nullptr;
}

#include "waylandclipboard.moc"
