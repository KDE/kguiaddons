# SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Quick)

# Blur Hash image provider

ecm_add_qml_module(kguiaddonsqmlprivate
    URI "org.kde.guiaddons.privateimageprovider" # to not import this module directly
)
target_sources(kguiaddonsqmlprivate PRIVATE
    blurhashimageprovider.cpp
    blurhashimageprovider.h
    privateplugin.cpp
    privateplugin.h
)

target_link_libraries(kguiaddonsqmlprivate PUBLIC
    Qt6::Qml Qt6::Quick KF6GuiAddons
)

ecm_finalize_qml_module(kguiaddonsqmlprivate DESTINATION ${KDE_INSTALL_QMLDIR})

# Public api

ecm_add_qml_module(kguiaddonsqml
    URI "org.kde.guiaddons"
    GENERATE_PLUGIN_SOURCE
    INSTALLED_PLUGIN_TARGET KF6::kguiaddonsqml
    IMPORTS
        "org.kde.guiaddons.privateimageprovider"
)
target_sources(kguiaddonsqml PRIVATE types.h)
target_link_libraries(kguiaddonsqml PRIVATE KF6GuiAddons Qt6::Qml)
ecm_finalize_qml_module(kguiaddonsqml EXPORT KF6GuiAddonsTargets)
