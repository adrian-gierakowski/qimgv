enable_testing()
find_package(Qt6 REQUIRED COMPONENTS Test Widgets)

include_directories(${CMAKE_SOURCE_DIR}/qimgv)

add_executable(unit_tests test_mapoverlay.cpp ../gui/overlays/mapoverlay.cpp)
target_link_libraries(unit_tests PRIVATE Qt6::Test Qt6::Widgets)
target_compile_definitions(unit_tests PRIVATE QT_WIDGETS_LIB QT_GUI_LIB QT_CORE_LIB)

add_test(NAME QUI_TEST COMMAND unit_tests)
