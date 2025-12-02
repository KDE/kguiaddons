// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#include "ksysteminhibitor.h"

#if defined(WITH_DBUS)
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <private/qdesktopunixservices_p.h>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#endif

#include "kguiaddons_debug.h"

using namespace Qt::StringLiterals;

#if defined(WITH_DBUS)
namespace
{
void release(const QDBusObjectPath &path)
{
    QDBusMessage message = QDBusMessage::createMethodCall(u"org.freedesktop.portal.Desktop"_s, path.path(), u"org.freedesktop.portal.Request"_s, u"Close"_s);
    QDBusConnection::sessionBus().asyncCall(message);
};
} // namespace
#endif

class KSystemInhibitorPrivate
{
public:
#if defined(WITH_DBUS)
    std::optional<QDBusObjectPath> m_inhibition;
#endif
};

KSystemInhibitor::KSystemInhibitor(const QString &reason, Types types, QWindow *window, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<KSystemInhibitorPrivate>())
{
#if defined(WITH_DBUS)
    QDBusMessage message = QDBusMessage::createMethodCall(u"org.freedesktop.portal.Desktop"_s,
                                                          u"/org/freedesktop/portal/desktop"_s,
                                                          u"org.freedesktop.portal.Inhibit"_s,
                                                          u"Inhibit"_s);

    const QString windowIdentifier = [window] {
        if (window) {
            auto services = QGuiApplicationPrivate::platformIntegration()->services();
            if (auto unixServices = dynamic_cast<QDesktopUnixServices *>(services)) {
                return unixServices->portalWindowIdentifier(window);
            }
        }
        return QString();
    }();
    message << windowIdentifier << static_cast<unsigned>(types.toInt()) << QVariantMap({{QLatin1String("reason"), reason}});

    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), qApp);
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     qApp,
                     [that = QPointer(this) /* we bind to the lifetime of qApp, guard this!*/](QDBusPendingCallWatcher *watcher) {
                         watcher->deleteLater();
                         QDBusPendingReply<QDBusObjectPath> reply = *watcher;
                         if (reply.isError()) {
                             qCWarning(KGUIADDONS_LOG) << "KSystemInhibitor Error: " << reply.error().message();
                         } else if (that) {
                             that->d->m_inhibition.emplace(reply.value());
                         } else { // KSystemInhibitor got deleted before we could store the inhibition, release it immediately.
                             release(reply.value());
                         }
                     });
#endif
}

KSystemInhibitor::~KSystemInhibitor()
{
#if defined(WITH_DBUS)
    if (d->m_inhibition.has_value()) {
        release(d->m_inhibition.value());
    }
#endif
}
