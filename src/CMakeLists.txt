
set(kguiaddons_LIB_SRCS
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
 util/urlhandler.cpp
)

set (kmodifierkeyinfoprovider_impl "dummy")
if (NOT APPLE AND X11_FOUND AND X11_Xkb_FOUND AND XCB_XCB_FOUND)
    find_package(Qt5X11Extras ${REQUIRED_QT_VERSION} NO_MODULE)
    if (Qt5X11Extras_FOUND)
        include_directories (
            ${X11_Xkb_INCLUDE_PATH}
            ${X11_Xlib_INCLUDE_PATH}
            ${XCB_XCB_INCLUDE_DIR}
        )
        set (kmodifierkeyinfoprovider_impl "x11")
    endif()
endif ()

set (kguiaddons_LIB_SRCS ${kguiaddons_LIB_SRCS} util/kmodifierkeyinfoprovider_${kmodifierkeyinfoprovider_impl}.cpp)

add_library(KF5GuiAddons ${kguiaddons_LIB_SRCS})

generate_export_header(KF5GuiAddons BASE_NAME KGuiAddons)
add_library(KF5::GuiAddons ALIAS KF5GuiAddons)

set(kguiaddons_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/colors
  ${CMAKE_CURRENT_SOURCE_DIR}/fonts
  ${CMAKE_CURRENT_SOURCE_DIR}/text
  ${CMAKE_CURRENT_SOURCE_DIR}/util
  )
target_include_directories(KF5GuiAddons PUBLIC "$<BUILD_INTERFACE:${kguiaddons_INCLUDES}>")
target_include_directories(KF5GuiAddons INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KGuiAddons>" )
target_compile_definitions(KF5GuiAddons INTERFACE "$<INSTALL_INTERFACE:KGUIADDONS_LIB>")
target_link_libraries(KF5GuiAddons PUBLIC Qt5::Gui)

if (kmodifierkeyinfoprovider_impl STREQUAL "x11")
    target_link_libraries(KF5GuiAddons LINK_PRIVATE ${X11_LIBRARIES} ${XCB_XCB_LIBRARY} Qt5::X11Extras)
endif()

set_target_properties(KF5GuiAddons PROPERTIES VERSION   ${KGUIADDONS_VERSION_STRING}
                                              SOVERSION ${KGUIADDONS_SOVERSION}
                                              EXPORT_NAME GuiAddons
)

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

  RELATIVE util
  REQUIRED_HEADERS KGuiAddons_HEADERS
)

install(TARGETS KF5GuiAddons EXPORT KF5GuiAddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_export.h
  util/klocalimagecacheimpl.h # implementation detail, no forwarding header
  ${KGuiAddons_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/KGuiAddons COMPONENT Devel
)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KGuiAddons LIB_NAME KF5GuiAddons DEPS "gui" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
