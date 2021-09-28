remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMMarkAsTest)

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test Widgets)

macro(kguiaddons_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt${QT_MAJOR_VERSION}::Widgets KF5::GuiAddons)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kguiaddons_executable_tests(
  kcolorcollectiontest
  kmodifierkeyinfotest
  openurltest
  kcursorsavergui_test
  kcolorschemewatchertest
)
