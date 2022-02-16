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
)

ecm_qt_declare_logging_category(KF5GuiAddons
    HEADER kguiaddons_debug.h
    IDENTIFIER KGUIADDONS_LOG
    CATEGORY_NAME kf.guiaddons
    DESCRIPTION "KGuiAddons"
    EXPORT KGUIADDONS
)

if(WITH_WAYLAND)
    if (TARGET Qt6::WaylandClient)
        qt_generate_wayland_protocol_client_sources(KF5GuiAddons
            FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/recorder/keyboard-shortcuts-inhibit-unstable-v1.xml"
                "${CMAKE_CURRENT_SOURCE_DIR}/systemclipboard/wlr-data-control-unstable-v1.xml"
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
        target_sources(KF5GuiAddons PRIVATE
            ${wayland_SRCS}
        )
    endif()
    target_sources(KF5GuiAddons PRIVATE
        recorder/waylandinhibition.cpp
        systemclipboard/waylandclipboard.cpp
    )
    target_compile_definitions(KF5GuiAddons PRIVATE WITH_WAYLAND)
    target_link_libraries(KF5GuiAddons PRIVATE Qt${QT_MAJOR_VERSION}::GuiPrivate Qt${QT_MAJOR_VERSION}::WaylandClient Wayland::Client)
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

if (WITH_X11 AND (TARGET Qt5::X11Extras OR TARGET Qt6::Gui))
    add_library(kmodifierkey_xcb MODULE util/kmodifierkeyinfoprovider_xcb.cpp)
    install(TARGETS kmodifierkey_xcb DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/kguiaddons/kmodifierkey/)

    if (TARGET Qt5::X11Extras)
        set(_qt_x11_libs Qt5::X11Extras)
    elseif (TARGET Qt6::Gui)
        set(_qt_x11_libs Qt6::GuiPrivate) # qtx11extras_p.h
    endif()
    target_link_libraries(kmodifierkey_xcb PRIVATE X11::Xkb XCB::XCB KF5::GuiAddons ${_qt_x11_libs})
endif()

ecm_generate_headers(KGuiAddons_HEADERS
  HEADER_NAMES
  KColorUtils
  KColorCollection
  KColorMimeData

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

install(TARGETS KF5GuiAddons EXPORT KF5GuiAddonsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
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
