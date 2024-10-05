# SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

ecm_add_qml_module(kguiaddonsqml URI "org.kde.guiaddons" GENERATE_PLUGIN_SOURCE INSTALLED_PLUGIN_TARGET KF6::kguiaddonsqml)
target_sources(kguiaddonsqml PRIVATE types.h)
target_link_libraries(kguiaddonsqml PRIVATE KF6GuiAddons Qt6::Qml)
ecm_finalize_qml_module(kguiaddonsqml EXPORT KF6GuiAddonsTargets)
