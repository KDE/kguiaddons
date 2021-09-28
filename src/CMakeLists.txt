configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_library(KF5GuiAddons)
add_library(KF5::GuiAddons ALIAS KF5GuiAddons)

set_target_properties(KF5GuiAddons PROPERTIES
    VERSION     ${KGUIADDONS_VERSION}
    SOVERSION   ${KGUIADDONS_SOVERSION}
    EXPORT_NAME GuiAddons
)

target_sources(KF5GuiAddons PRIVATE
 colors/kcolorspaces.cpp
 colors/kcolorutils.cpp
 colors/kcolorcollection.cpp
 colors/kcolormimedata.cpp
 colors/kcolorschemewatcher.cpp
 colors/kcolorschemewatcherbackend.cpp
 text/kdatevalidator.cpp
 text/kwordwrap.cpp
 fonts/kfontutils.cpp
 util/kiconutils.cpp
 util/klocalimagecacheimpl.cpp
 util/kmodifierkeyinfo.cpp
 util/kmodifierkeyinfoprovider.cpp
 util/kurlhandler_p.cpp
 util/kcursorsaver.cpp
 recorder/keyboardgrabber.cpp
 recorder/keysequencerecorder.cpp
 systemclipboard/qtclipboard.cpp
 systemclipboard/ksystemclipboard.cpp

 colors/kcolorspaces_p.h
 colors/kcolorutils.h
 colors/kcolorcollection.h
 colors/kcolormimedata.h
 text/kdatevalidator.h
 text/kwordwrap.h
 fonts/kfontutils.h
 util/kiconutils.h
 util/klocalimagecacheimpl.h
 util/kmodifierkeyinfo.h
 util/kmodifierkeyinfoprovider_p.h
 util/kurlhandler_p.h
 util/kcursorsaver.h
 recorder/keyboardgrabber_p.h
 recorder/keysequencerecorder.h
 systemclipboard/qtclipboard_p.h
 systemclipboard/ksystemclipboard.h
)

ecm_qt_declare_logging_category(KF5GuiAddons
    HEADER kguiaddons_debug.h
    IDENTIFIER KGUIADDONS_LOG
    CATEGORY_NAME kf.guiaddons
    DESCRIPTION "KGuiAddons"
    EXPORT KGUIADDONS
)

if(WIN32)
    target_sources(KF5GuiAddons PRIVATE colors/kcolorschemewatcher_win.cpp)
    target_link_libraries(KF5GuiAddons PRIVATE advapi32)
endif()

if(WITH_DBUS)
    target_sources(KF5GuiAddons PRIVATE colors/kcolorschemewatcher_xdg.cpp)
    target_link_libraries(KF5GuiAddons PRIVATE Qt${QT_MAJOR_VERSION}::DBus)
endif()

if(WITH_WAYLAND)
    if (TARGET Qt6::WaylandClient)
        qt_generate_wayland_protocol_client_sources(KF5GuiAddons
            FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/recorder/keyboard-shortcuts-inhibit-unstable-v1.xml"
                "${CMAKE_CURRENT_SOURCE_DIR}/systemclipboard/wlr-data-control-unstable-v1.xml"
                "${PLASMA_WAYLAND_PROTOCOLS_DIR}/keystate.xml"
        )
    else()
        set(wayland_SRCS)
        ecm_add_qtwayland_client_protocol(wayland_SRCS
            PROTOCOL recorder/keyboard-shortcuts-inhibit-unstable-v1.xml
            BASENAME keyboard-shortcuts-inhibit-unstable-v1
        )
        ecm_add_qtwayland_client_protocol(wayland_SRCS
            PROTOCOL  systemclipboard/wlr-data-control-unstable-v1.xml
            BASENAME wlr-data-control-unstable-v1
        )
        ecm_add_qtwayland_client_protocol(wayland_SRCS
            PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/keystate.xml
            BASENAME keystate
        )
        target_sources(KF5GuiAddons PRIVATE
            ${wayland_SRCS}
        )
    endif()
    target_sources(KF5GuiAddons PRIVATE
        recorder/waylandinhibition.cpp
        systemclipboard/waylandclipboard.cpp
        util/kmodifierkeyinfoprovider_wayland.cpp
        recorder/waylandinhibition_p.h
        systemclipboard/waylandclipboard_p.h
        util/kmodifierkeyinfoprovider_wayland.h
    )
    target_compile_definitions(KF5GuiAddons PRIVATE WITH_WAYLAND)
    target_link_libraries(KF5GuiAddons PRIVATE Qt${QT_MAJOR_VERSION}::GuiPrivate Qt${QT_MAJOR_VERSION}::WaylandClient Wayland::Client)
endif()

if(WITH_X11)
    target_sources(KF5GuiAddons PRIVATE util/kmodifierkeyinfoprovider_xcb.cpp util/kmodifierkeyinfoprovider_xcb.h)
    if (TARGET Qt5::X11Extras)
        target_link_libraries(KF5GuiAddons PRIVATE Qt5::X11Extras)
    endif()
    target_link_libraries(KF5GuiAddons PRIVATE X11::Xkb XCB::XCB)
endif()

generate_export_header(KF5GuiAddons BASE_NAME KGuiAddons)

set(kguiaddons_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/colors
  ${CMAKE_CURRENT_SOURCE_DIR}/fonts
  ${CMAKE_CURRENT_SOURCE_DIR}/text
  ${CMAKE_CURRENT_SOURCE_DIR}/util
  ${CMAKE_CURRENT_SOURCE_DIR}/recorder
  ${CMAKE_CURRENT_SOURCE_DIR}/systemclipboard
  )
target_include_directories(KF5GuiAddons PUBLIC "$<BUILD_INTERFACE:${kguiaddons_INCLUDES}>")
target_include_directories(KF5GuiAddons INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KGuiAddons>" )
target_compile_definitions(KF5GuiAddons INTERFACE "$<INSTALL_INTERFACE:KGUIADDONS_LIB>")
target_link_libraries(KF5GuiAddons PUBLIC Qt${QT_MAJOR_VERSION}::Gui)

ecm_generate_headers(KGuiAddons_HEADERS
  HEADER_NAMES
  KColorUtils
  KColorCollection
  KColorMimeData
  KColorSchemeWatcher

  RELATIVE colors
  REQUIRED_HEADERS KGuiAddons_HEADERS
)
ecm_generate_headers(KGuiAddons_HEADERS
  HEADER_NAMES
  KDateValidator
  KWordWrap

  RELATIVE text
  REQUIRED_HEADERS KGuiAddons_HEADERS
)
ecm_generate_headers(KGuiAddons_HEADERS
  HEADER_NAMES
  KFontUtils

  RELATIVE fonts
  REQUIRED_HEADERS KGuiAddons_HEADERS
)
ecm_generate_headers(KGuiAddons_HEADERS
  HEADER_NAMES
  KIconUtils
  KImageCache
  KModifierKeyInfo
  KCursorSaver

  RELATIVE util
  REQUIRED_HEADERS KGuiAddons_HEADERS
)

ecm_generate_headers(KGuiAddons_HEADERS
    HEADER_NAMES
    KeySequenceRecorder

    RELATIVE recorder
    REQUIRED_HEADERS KGuiAddons_HEADERS
)

#add_subdirectory(systemclipboard)

ecm_generate_headers(KGuiAddons_HEADERS
    HEADER_NAMES
    KSystemClipboard

    RELATIVE systemclipboard
    REQUIRED_HEADERS KGuiAddons_HEADERS
)

install(TARGETS KF5GuiAddons EXPORT KF5GuiAddonsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_export.h
  util/kmodifierkeyinfoprovider_p.h
  util/klocalimagecacheimpl.h # implementation detail, no forwarding header
  ${KGuiAddons_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KGuiAddons COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF5GuiAddons_QCH
        NAME KGuiAddons
        BASE_NAME KF5GuiAddons
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KGuiAddons_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Gui_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
            ${kguiaddons_INCLUDES}
        BLANK_MACROS
            KGUIADDONS_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KGuiAddons LIB_NAME KF5GuiAddons DEPS "gui" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KGuiAddons)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

ecm_qt_install_logging_categories(
    EXPORT KGUIADDONS
    FILE kguiaddons.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

if(BUILD_TESTING)
    add_executable(pasteclient systemclipboard/tests/paste.cpp)

    target_link_libraries(pasteclient
        KF5GuiAddons
    )
endif()

add_subdirectory(geo-scheme-handler)
