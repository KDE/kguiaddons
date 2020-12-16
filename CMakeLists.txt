cmake_minimum_required(VERSION 3.5)

set(KF5_VERSION "5.78.0") # handled by release scripts
project(KGuiAddons VERSION ${KF5_VERSION})

include(FeatureSummary)
find_package(ECM 5.77.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)



option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

if (UNIX AND NOT ANDROID AND NOT APPLE)
    option(WITH_WAYLAND "Build with support for KeySequenceEditor inhibiting shortcuts on Wayland" ON)
    add_feature_info(WAYLAND ${WITH_WAYLAND} "KeySequenceEditor inhibiting shortcuts on Wayland")
else()
    set(WITH_WAYLAND OFF)
endif()

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

set(REQUIRED_QT_VERSION 5.13.0)
if (WITH_WAYLAND)
    set(_qtgui_find_components COMPONENTS Private)
endif()
# QtGui must be found becore Qt5WaylandClient
find_package(Qt5Gui ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE ${_qtgui_find_components})

if (NOT APPLE AND NOT WIN32)
    find_package(X11 MODULE)
    find_package(XCB MODULE COMPONENTS XCB)
endif()

if (WITH_WAYLAND)
    find_package(Qt5WaylandClient ${REQUIRED_QT_VERSION} NO_MODULE)
    find_package(QtWaylandScanner REQUIRED)
    find_package(Wayland 1.9 REQUIRED Client)
endif()

include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)

ecm_setup_version(PROJECT VARIABLE_PREFIX KGUIADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5GuiAddonsConfigVersion.cmake"
                        SOVERSION 5)


add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
add_definitions(-DQT_NO_FOREACH)
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5GuiAddons")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5GuiAddons_QCH
        FILE KF5GuiAddonsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5GuiAddonsQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5GuiAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5GuiAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5GuiAddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5GuiAddonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )


install(EXPORT KF5GuiAddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5GuiAddonsTargets.cmake NAMESPACE KF5:: COMPONENT Devel)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
