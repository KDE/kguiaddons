cmake_minimum_required(VERSION 3.0)

set(KF5_VERSION "5.32.0") # handled by release scripts
project(KGuiAddons VERSION ${KF5_VERSION})

include(FeatureSummary)
find_package(ECM 5.32.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
set(REQUIRED_QT_VERSION 5.6.0)
find_package(Qt5Gui ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

if (NOT APPLE AND NOT WIN32)
    find_package(X11 MODULE)
    find_package(XCB MODULE COMPONENTS XCB)
endif()


include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

ecm_setup_version(PROJECT VARIABLE_PREFIX KGUIADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5GuiAddonsConfigVersion.cmake"
                        SOVERSION 5)

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5GuiAddons")

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
