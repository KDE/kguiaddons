# SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

ecm_add_qml_module(kguiaddonsqml URI "org.kde.guiaddons" CLASS_NAME KGuiAddonsPlugin INSTALLED_PLUGIN_TARGET KF6::kguiaddonsqml)
target_sources(kguiaddonsqml PRIVATE
    kcolorutilssingleton.cpp
    kguiaddonsplugin.cpp
    types.h
    systeminhibitor.cpp
)
# Quick is needed for type information on (e.g.) QWindow
target_link_libraries(kguiaddonsqml PRIVATE KF6GuiAddons Qt6::Qml Qt6::Quick)
ecm_finalize_qml_module(kguiaddonsqml EXPORT KF6GuiAddonsTargets)
