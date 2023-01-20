find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_tests(
  kwordwraptest.cpp
  kcolorutilstest.cpp
  kiconutilstest.cpp
  kcursorsavertest.cpp
  keysequencerecordertest.cpp
  kurlhandlertest.cpp
  LINK_LIBRARIES KF6::GuiAddons Qt${QT_MAJOR_VERSION}::Test
)
ecm_add_tests(kgeourihandlertest.cpp LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test)
