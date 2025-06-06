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

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "qwayland-wayland.h"
#include "qwayland-ext-data-control-v1.h"

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

class DataControlDeviceManager : public QWaylandClientExtensionTemplate<DataControlDeviceManager>, public QtWayland::ext_data_control_manager_v1
{
    Q_OBJECT
public:
    DataControlDeviceManager()
        : QWaylandClientExtensionTemplate<DataControlDeviceManager>(1)
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

class DataControlOffer : public QMimeData, public QtWayland::ext_data_control_offer_v1
{
    Q_OBJECT
public:
    DataControlOffer(struct ::ext_data_control_offer_v1 *id)
        : QtWayland::ext_data_control_offer_v1(id)
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
    void ext_data_control_offer_v1_offer(const QString &mime_type) override
    {
        if (!m_receivedFormats.contains(mime_type)) {
            m_receivedFormats << mime_type;
        }
    }

    QVariant retrieveData(const QString &mimeType, QMetaType type) const override;

private:
    /* reads data from a file descriptor with a timeout of 1 second
     *  true if data is read successfully
     */
    static bool readData(int fd, QByteArray &data, const QString &mimeType);
    QStringList m_receivedFormats;
    mutable QHash<QString, QVariant> m_data;
};

QVariant DataControlOffer::retrieveData(const QString &mimeType, QMetaType type) const
{
    Q_UNUSED(type);

    auto it = m_data.constFind(mimeType);
    if (it != m_data.constEnd())
        return *it;

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

    auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    auto display = waylandApp->display();

    wl_display_flush(display);

    QFile readPipe;
    if (readPipe.open(pipeFds[0], QIODevice::ReadOnly)) {
        QByteArray data;
        if (readData(pipeFds[0], data, mime)) {
            close(pipeFds[0]);

            if (mimeType == applicationQtXImageLiteral()) {
                QImage img = QImage::fromData(data, mime.mid(mime.indexOf(QLatin1Char('/')) + 1).toLatin1().toUpper().data());
                if (!img.isNull()) {
                    m_data.insert(mimeType, img);
                    return img;
                }
            } else if (data.size() > 1 && mimeType == u"text/uri-list") {
                const auto urls = data.split('\n');
                QVariantList list;
                list.reserve(urls.size());
                for (const QByteArray &s : urls) {
                    if (QUrl url(QUrl::fromEncoded(QByteArrayView(s).trimmed())); url.isValid()) {
                        list.emplace_back(std::move(url));
                    }
                }
                m_data.insert(mimeType, list);
                return list;
            }
            m_data.insert(mimeType, data);
            return data;
        }
        close(pipeFds[0]);
    }

    return QVariant();
}

bool DataControlOffer::readData(int fd, QByteArray &data, const QString &mimeType)
{
    pollfd pfds[1];
    pfds[0].fd = fd;
    pfds[0].events = POLLIN;

    while (true) {
        const int ready = poll(pfds, 1, 1000);
        if (ready < 0) {
            if (errno != EINTR) {
                qWarning("DataControlOffer: poll() failed for mimeType %s: %s", qPrintable(mimeType), strerror(errno));
                return false;
            }
        } else if (ready == 0) {
            qWarning("DataControlOffer: timeout reading from pipe for mimeType %s", qPrintable(mimeType));
            return false;
        } else {
            char buf[4096];
            int n = read(fd, buf, sizeof buf);

            if (n < 0) {
                qWarning("DataControlOffer: read() failed for mimeType %s: %s", qPrintable(mimeType), strerror(errno));
                return false;
            } else if (n == 0) {
                return true;
            } else if (n > 0) {
                data.append(buf, n);
            }
        }
    }
}

class DataControlSource : public QObject, public QtWayland::ext_data_control_source_v1
{
    Q_OBJECT
public:
    DataControlSource(struct ::ext_data_control_source_v1 *id, QMimeData *mimeData);
    DataControlSource() = default;
    ~DataControlSource()
    {
        destroy();
    }

    QMimeData *mimeData()
    {
        return m_mimeData.get();
    }
    std::unique_ptr<QMimeData> releaseMimeData()
    {
        return std::move(m_mimeData);
    }

Q_SIGNALS:
    void cancelled();

protected:
    void ext_data_control_source_v1_send(const QString &mime_type, int32_t fd) override;
    void ext_data_control_source_v1_cancelled() override;

private:
    std::unique_ptr<QMimeData> m_mimeData;
};

DataControlSource::DataControlSource(struct ::ext_data_control_source_v1 *id, QMimeData *mimeData)
    : QtWayland::ext_data_control_source_v1(id)
    , m_mimeData(mimeData)
{
    const auto formats = mimeData->formats();
    for (const QString &format : formats) {
        offer(format);
    }
    if (mimeData->hasText()) {
        // ensure GTK applications get this mimetype to avoid them discarding the offer
        offer(QStringLiteral("text/plain;charset=utf-8"));
    }

    if (mimeData->hasImage()) {
        const QStringList imageFormats = imageWriteMimeFormats();
        for (const QString &imageFormat : imageFormats) {
            if (!formats.contains(imageFormat)) {
                offer(imageFormat);
            }
        }
    }
}

void DataControlSource::ext_data_control_source_v1_send(const QString &mime_type, int32_t fd)
{
    QString send_mime_type = mime_type;
    if (send_mime_type == QStringLiteral("text/plain;charset=utf-8")) {
        // if we get a request on the fallback mime, send the data from the original mime type
        send_mime_type = QStringLiteral("text/plain");
    }

    QByteArray ba;
    if (m_mimeData->hasImage()) {
        // adapted from QInternalMimeData::renderDataHelper
        if (mime_type == applicationQtXImageLiteral()) {
            QImage image = qvariant_cast<QImage>(m_mimeData->imageData());
            QBuffer buf(&ba);
            buf.open(QBuffer::WriteOnly);
            // would there not be PNG ??
            image.save(&buf, "PNG");

        } else if (mime_type.startsWith(QLatin1String("image/"))) {
            QImage image = qvariant_cast<QImage>(m_mimeData->imageData());
            QBuffer buf(&ba);
            buf.open(QBuffer::WriteOnly);
            image.save(&buf, mime_type.mid(mime_type.indexOf(QLatin1Char('/')) + 1).toLatin1().toUpper().data());
        }
        // end adapted
    } else {
        ba = m_mimeData->data(send_mime_type);
    }

    QFile c;
    if (!c.open(fd, QFile::WriteOnly, QFile::AutoCloseHandle)) {
        return;
    }
    // Create a sigpipe handler that does nothing, or clients may be forced to terminate
    // if the pipe is closed in the other end.
    struct sigaction action, oldAction;
    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGPIPE, &action, &oldAction);
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags & O_NONBLOCK) {
        fcntl(fd, F_SETFL, flags & ~O_NONBLOCK); // Unset O_NONBLOCK to fix pasting to XWayland windows
    }
    const qint64 written = c.write(ba);
    sigaction(SIGPIPE, &oldAction, nullptr);

    if (written != ba.size()) {
        qWarning() << "Failed to send all clipobard data; sent" << written << "bytes out of" << ba.size();
    }
}

void DataControlSource::ext_data_control_source_v1_cancelled()
{
    Q_EMIT cancelled();
}

class DataControlDevice : public QObject, public QtWayland::ext_data_control_device_v1
{
    Q_OBJECT
public:
    DataControlDevice(struct ::ext_data_control_device_v1 *id)
        : QtWayland::ext_data_control_device_v1(id)
    {
    }

    ~DataControlDevice()
    {
        destroy();
    }

    void setSelection(std::unique_ptr<DataControlSource> selection);
    QMimeData *receivedSelection()
    {
        return m_receivedSelection.get();
    }
    QMimeData *selection()
    {
        return m_selection ? m_selection->mimeData() : nullptr;
    }

    void setPrimarySelection(std::unique_ptr<DataControlSource> selection);
    QMimeData *receivedPrimarySelection()
    {
        return m_receivedPrimarySelection.get();
    }
    QMimeData *primarySelection()
    {
        return m_primarySelection ? m_primarySelection->mimeData() : nullptr;
    }

Q_SIGNALS:
    void receivedSelectionChanged();
    void selectionChanged();

    void receivedPrimarySelectionChanged();
    void primarySelectionChanged();

protected:
    void ext_data_control_device_v1_data_offer(struct ::ext_data_control_offer_v1 *id) override
    {
        // this will become memory managed when we retrieve the selection event
        // a compositor calling data_offer without doing that would be a bug
        new DataControlOffer(id);
    }

    void ext_data_control_device_v1_selection(struct ::ext_data_control_offer_v1 *id) override
    {
        if (!id) {
            m_receivedSelection.reset();
        } else {
            auto derivated = QtWayland::ext_data_control_offer_v1::fromObject(id);
            auto offer = dynamic_cast<DataControlOffer *>(derivated); // dynamic because of the dual inheritance
            m_receivedSelection.reset(offer);
        }
        Q_EMIT receivedSelectionChanged();
    }

    void ext_data_control_device_v1_primary_selection(struct ::ext_data_control_offer_v1 *id) override
    {
        if (!id) {
            m_receivedPrimarySelection.reset();
        } else {
            auto derivated = QtWayland::ext_data_control_offer_v1::fromObject(id);
            auto offer = dynamic_cast<DataControlOffer *>(derivated); // dynamic because of the dual inheritance
            m_receivedPrimarySelection.reset(offer);
        }
        Q_EMIT receivedPrimarySelectionChanged();
    }

private:
    std::unique_ptr<DataControlSource> m_selection; // selection set locally
    std::unique_ptr<DataControlOffer> m_receivedSelection; // latest selection set from externally to here

    std::unique_ptr<DataControlSource> m_primarySelection; // selection set locally
    std::unique_ptr<DataControlOffer> m_receivedPrimarySelection; // latest selection set from externally to here
    friend WaylandClipboard;
};

void DataControlDevice::setSelection(std::unique_ptr<DataControlSource> selection)
{
    set_selection(selection->object());

    // Note the previous selection is destroyed after the set_selection request.
    m_selection = std::move(selection);
    connect(m_selection.get(), &DataControlSource::cancelled, this, [this]() {
        m_selection.reset();
    });

    Q_EMIT selectionChanged();
}

void DataControlDevice::setPrimarySelection(std::unique_ptr<DataControlSource> selection)
{
    set_primary_selection(selection->object());

    // Note the previous selection is destroyed after the set_primary_selection request.
    m_primarySelection = std::move(selection);
    connect(m_primarySelection.get(), &DataControlSource::cancelled, this, [this]() {
        m_primarySelection.reset();
    });

    Q_EMIT primarySelectionChanged();
}
class Keyboard;
// We are binding to Seat/Keyboard manually because we want to react to gaining focus but inside Qt the events are Qt and arrive to late
class KeyboardFocusWatcher : public QWaylandClientExtensionTemplate<KeyboardFocusWatcher>, public QtWayland::wl_seat
{
    Q_OBJECT
public:
    KeyboardFocusWatcher()
        : QWaylandClientExtensionTemplate(5)
    {
        initialize();
        auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
        auto display = waylandApp->display();
        // so we get capabilities
        wl_display_roundtrip(display);
    }
    ~KeyboardFocusWatcher() override
    {
        if (isActive()) {
            release();
        }
    }
    void seat_capabilities(uint32_t capabilities) override
    {
        const bool hasKeyboard = capabilities & capability_keyboard;
        if (hasKeyboard && !m_keyboard) {
            m_keyboard = std::make_unique<Keyboard>(get_keyboard(), *this);
        } else if (!hasKeyboard && m_keyboard) {
            m_keyboard.reset();
        }
    }
    bool hasFocus() const
    {
        return m_focus;
    }
Q_SIGNALS:
    void keyboardEntered();

private:
    friend Keyboard;
    bool m_focus = false;
    std::unique_ptr<Keyboard> m_keyboard;
};

class Keyboard : public QtWayland::wl_keyboard
{
public:
    Keyboard(::wl_keyboard *keyboard, KeyboardFocusWatcher &seat)
        : wl_keyboard(keyboard)
        , m_seat(seat)
    {
    }
    ~Keyboard()
    {
        release();
    }

private:
    void keyboard_enter([[maybe_unused]] uint32_t serial, [[maybe_unused]] wl_surface *surface, [[maybe_unused]] wl_array *keys) override
    {
        m_seat.m_focus = true;
        Q_EMIT m_seat.keyboardEntered();
    }
    void keyboard_leave([[maybe_unused]] uint32_t serial, [[maybe_unused]] wl_surface *surface) override
    {
        m_seat.m_focus = false;
    }
    KeyboardFocusWatcher &m_seat;
};

WaylandClipboard::WaylandClipboard(QObject *parent)
    : KSystemClipboard(parent)
    , m_keyboardFocusWatcher(new KeyboardFocusWatcher)
    , m_manager(new DataControlDeviceManager)
{
    connect(m_manager.get(), &DataControlDeviceManager::activeChanged, this, [this]() {
        if (m_manager->isActive()) {
            auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
            if (!waylandApp) {
                return;
            }
            auto seat = waylandApp->seat();

            if (!seat) {
                return;
            }
            m_device.reset(new DataControlDevice(m_manager->get_data_device(seat)));

            connect(m_device.get(), &DataControlDevice::receivedSelectionChanged, this, [this]() {
                // When our source is still valid, so the offer is for setting it or we emit changed when it is cancelled
                if (!m_device->selection()) {
                    Q_EMIT changed(QClipboard::Clipboard);
                }
            });
            connect(m_device.get(), &DataControlDevice::selectionChanged, this, [this]() {
                Q_EMIT changed(QClipboard::Clipboard);
            });

            connect(m_device.get(), &DataControlDevice::receivedPrimarySelectionChanged, this, [this]() {
                // When our source is still valid, so the offer is for setting it or we emit changed when it is cancelled
                if (!m_device->primarySelection()) {
                    Q_EMIT changed(QClipboard::Selection);
                }
            });
            connect(m_device.get(), &DataControlDevice::primarySelectionChanged, this, [this]() {
                Q_EMIT changed(QClipboard::Selection);
            });

        } else {
            m_device.reset();
        }
    });

    m_manager->instantiate();
}

WaylandClipboard::~WaylandClipboard() = default;

WaylandClipboard *WaylandClipboard::create(QObject *parent)
{
    auto clipboard = new WaylandClipboard(parent);
    if (clipboard->isValid()) {
        return clipboard;
    }
    delete clipboard;
    return nullptr;
}

bool WaylandClipboard::isValid()
{
    return m_manager && m_manager->isInitialized();
}

void WaylandClipboard::setMimeData(QMimeData *mime, QClipboard::Mode mode)
{
    if (!m_device) {
        return;
    }

    // roundtrip to have accurate focus state when losing focus but setting mime data before processing wayland events.
    auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    auto display = waylandApp->display();
    wl_display_roundtrip(display);

    // If the application is focused, use the normal mechanism so a future paste will not deadlock itselfs
    if (m_keyboardFocusWatcher->hasFocus()) {
        QGuiApplication::clipboard()->setMimeData(mime, mode);
        // if we short-circuit the ext_data_device, when we receive the data
        // we cannot identify ourselves as the owner
        // because of that we act like it's a synchronous action to not confuse klipper.
        wl_display_roundtrip(display);
        return;
    }
    // If not, set the clipboard once the app receives focus to avoid the deadlock
    connect(m_keyboardFocusWatcher.get(), &KeyboardFocusWatcher::keyboardEntered, this, &WaylandClipboard::gainedFocus, Qt::UniqueConnection);
    auto source = std::make_unique<DataControlSource>(m_manager->create_data_source(), mime);
    if (mode == QClipboard::Clipboard) {
        m_device->setSelection(std::move(source));
    } else if (mode == QClipboard::Selection) {
        m_device->setPrimarySelection(std::move(source));
    }
}

void WaylandClipboard::gainedFocus()
{
    disconnect(m_keyboardFocusWatcher.get(), &KeyboardFocusWatcher::keyboardEntered, this, nullptr);
    // QClipboard takes ownership of the QMimeData so we need to transfer and unset our selections
    if (auto &selection = m_device->m_selection) {
        std::unique_ptr<QMimeData> data = selection->releaseMimeData();
        selection.reset();
        QGuiApplication::clipboard()->setMimeData(data.release(), QClipboard::Clipboard);
    }
    if (auto &primarySelection = m_device->m_primarySelection) {
        std::unique_ptr<QMimeData> data = primarySelection->releaseMimeData();
        primarySelection.reset();
        QGuiApplication::clipboard()->setMimeData(data.release(), QClipboard::Selection);
    }
}

void WaylandClipboard::clear(QClipboard::Mode mode)
{
    if (!m_device) {
        return;
    }
    if (mode == QClipboard::Clipboard) {
        m_device->set_selection(nullptr);
        m_device->m_selection.reset();
    } else if (mode == QClipboard::Selection) {
        m_device->set_primary_selection(nullptr);
        m_device->m_primarySelection.reset();
    }
}

const QMimeData *WaylandClipboard::mimeData(QClipboard::Mode mode) const
{
    if (!m_device) {
        return nullptr;
    }

    // return our locally set selection if it's not cancelled to avoid copying data to ourselves
    if (mode == QClipboard::Clipboard) {
        if (m_device->selection()) {
            return m_device->selection();
        }
        // This application owns the clipboard via the regular data_device, use it so we don't block ourselves
        if (QGuiApplication::clipboard()->ownsClipboard()) {
            return QGuiApplication::clipboard()->mimeData(mode);
        }
        return m_device->receivedSelection();
    } else if (mode == QClipboard::Selection) {
        if (m_device->primarySelection()) {
            return m_device->primarySelection();
        }
        // This application owns the primary selection via the regular primary_selection_device, use it so we don't block ourselves
        if (QGuiApplication::clipboard()->ownsSelection()) {
            return QGuiApplication::clipboard()->mimeData(mode);
        }
        return m_device->receivedPrimarySelection();
    }
    return nullptr;
}

#include "waylandclipboard.moc"
