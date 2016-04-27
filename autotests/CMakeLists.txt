find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_tests(
  kwordwraptest.cpp
  kcolorutilstest.cpp
  kiconutilstest.cpp
  LINK_LIBRARIES KF5::GuiAddons Qt5::Test
)

if (PythonModuleGeneration_FOUND AND EXISTS ${CMAKE_BINARY_DIR}/prefix/lib/python2.7/dist-packages)
  add_test(NAME Py2${name} COMMAND python2 ${pythontest} ${CMAKE_BINARY_DIR}/prefix/lib/python2.7/dist-packages)
endif()

if (PythonModuleGeneration_FOUND AND EXISTS ${CMAKE_BINARY_DIR}/prefix/lib/python3.5/dist-packages)
  add_test(NAME Py3${name} COMMAND python3 ${pythontest} ${CMAKE_BINARY_DIR}/prefix/lib/python3.5/dist-packages)
endif()
