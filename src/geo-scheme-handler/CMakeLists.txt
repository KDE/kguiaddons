# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

add_executable(kde-geo-uri-handler
    kgeourihandler.cpp
    main.cpp
)
target_include_directories(kde-geo-uri-handler PRIVATE ${CMAKE_BINARY_DIR}) # for kguiaddons_version.h
target_link_libraries(kde-geo-uri-handler PRIVATE Qt${QT_MAJOR_VERSION}::Gui)

install(TARGETS kde-geo-uri-handler ${INSTALL_TARGETS_DEFAULT_ARGS})
install(
    FILES
        google-maps-geo-handler.desktop
        openstreetmap-geo-handler.desktop
        qwant-maps-geo-handler.desktop
        wheelmap-geo-handler.desktop
    DESTINATION
        ${KDE_INSTALL_APPDIR}
)
