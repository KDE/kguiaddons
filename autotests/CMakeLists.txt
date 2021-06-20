find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_tests(
  kwordwraptest.cpp
  kcolorutilstest.cpp
  kiconutilstest.cpp
  kcursorsavertest.cpp
  keysequencerecordertest.cpp
  kurlhandlertest.cpp
  LINK_LIBRARIES KF5::GuiAddons Qt5::Test
)
