/*
    SPDX-FileCopyrightText: 2026 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KDRAGUTILS_H
#define KDRAGUTILS_H

#include "kguiaddons_export.h"

#include <functional>

class QDrag;
class QIcon;
class QPixmap;
class QImage;
class QPaintDevice;
class QPainter;
class QPoint;
class QSize;

/*!
 * \namespace KDragUtils
 * \inmodule KGuiAddons
 *
 * \brief Provides utility functions for drag operations.
 */
namespace KDragUtils
{
/*!
 *  Drag object types.
 *
 *  \value CompleteObject A complete object, e.g. an image. Does not need to have high details in the drag pixmap.
 *  \value Selection A selection of a bigger object, e.g. part of a text. Ideally has full details in the drag pixmap, to confirm what content is dragged
 * exactly.
 *
 * \since 6.30
 */
enum class DragObjectType {
    CompleteObject,
    Selection,
};

/*!
 * The drag pixmap painting function signature.
 *
 * \a paintDevice is a pointer to the device to render the normal visual representation of the dragged object to.
 * The content is uninitialized. Any visual drag effects will be applied later internally.
 * \a pixmapSize is the size in logical pixels which is available on the device. It will be the display size
 * initially passed to \l setDragPixmap(QDrag *DragPixmapPaintFunction, QSize, QPoint, DragType), unless
 * the size has to be scaled down to fit into the available bounds.
 *
 * \sa setDragPixmap(QDrag *DragPixmapPaintFunction, QSize, QPoint, DragType)
 * \since 6.30
 */
using DragPixmapPaintFunction = std::function<bool(QPaintDevice *paintDevice, QSize pixmapSize)>;

/*!
 * Sets a ghost/shadow pixmap to a drag instance for the object (or its copy) being dragged.
 *
 * Such pixmap's purpose is to resemble closely the display of the object which is currently dragged and
 * which might be moved to another location or from which a copy is to be created, if the drag is
 * completed. The pixmap usually has some visual clue to reflect the in-process state, e.g. being
 * shown not fully opaque.
 *
 * To resemble a grab-like drag more closely, the relative position of the drag inside the display of the
 * object the drag is ddone from is used to calculate the hotspot to use with the drag pixmap.
 *
 * \a paintFunction is the callback method to call for rendering the pixmap on a prepared paint device.
 * \a displaySize is the size in logical pixels of the current display of the object which is to be dragged.
 * \a dragOffset is the offset in logical pixels of the drag center from the top-left position of the display
 * of the object to be dragged. It is used to set the hotspot property of the \a drag instance, taking into
 * account any scaling.
 * \a dragobjectType specifies the type of the dragged object. This is considered when estimating properties of the
 * final pixmap.
 *
 * Example usage:
 * \code
    auto *const drag = new QDrag(this);
    auto *const mimeData = createMimeData(object);
    drag->setMimeData(mimeData);

    const QRect boundingRect = renderer->boundingRect(object);
    auto painting = [renderer, object](QPaintDevice *paintDevice, QSize pixmapSize) -> bool {
        return renderer->render(paintDevice, pixmapSize, object);
    };
    const QSize displaySize = boundingRect.size();
    const QPoint dragOffset = dragStartPosition - boundingRect.topLeft();
    KDragUtils::setDragPixmap(drag, painting, displaySize, dragOffset, KDragUtils::DragObjectType::CompleteObject);

    drag->exec(Qt::CopyAction);
 * \endcode
 *
 * \since 6.30
 */
KGUIADDONS_EXPORT bool setDragPixmap(QDrag *drag, DragPixmapPaintFunction paintFunction, QSize displaySize, QPoint dragOffset, DragObjectType dragobjectType);

/*!
 * Sets a ghost/shadow pixmap to a drag instance for the object (or its copy) being dragged.
 *
 * Convenience overload taking an existing \a pixmap, The pixmap's logical size does not need to match
 * the actual \a displaySize, it will be rendered proportionally scaled as needed to fit the final drag pixmap.
 *
 * \since 6.30
 */
KGUIADDONS_EXPORT bool setDragPixmap(QDrag *drag, const QPixmap &pixmap, QSize displaySize, QPoint dragOffset, DragObjectType dragobjectType);

/*!
 * Sets a ghost/shadow pixmap to a drag instance for the object (or its copy) being dragged.
 *
 * Convenience overload taking an existing \a image, The image's logical size does not need to match
 * the actual \a displaySize, it will be rendered proportionally scaled as needed to fit the final drag pixmap.
 *
 * \since 6.30
 */
KGUIADDONS_EXPORT bool setDragPixmap(QDrag *drag, const QImage &image, QSize displaySize, QPoint dragOffset, DragObjectType dragobjectType);

/*!
 * Sets a ghost/shadow pixmap to a drag instance, using an icon as symbolic pixmap.
 *
 * The \a icon will be used to create the pixmap in a standard size
 * and set it to the \a drag instance, without any hotspot.
 *
 * \since 6.30
 */
KGUIADDONS_EXPORT bool setDragIcon(QDrag *drag, const QIcon &icon);
}

#endif
