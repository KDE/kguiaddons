find_package(Qt5 5.2.0 CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_tests(
  kwordwraptest.cpp
  kcolorutilstest.cpp
  kiconutilstest.cpp
  NAME_PREFIX "kguiaddons-"
  LINK_LIBRARIES KF5::GuiAddons Qt5::Test
)
