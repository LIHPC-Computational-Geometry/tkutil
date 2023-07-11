include (${CMAKE_SOURCE_DIR}/cmake/version.cmake)
include (${GUIToolkitsVariables_CMAKE_DIR}/common.cmake)
include (${GUIToolkitsVariables_CMAKE_DIR}/workarounds.cmake)

add_executable (encoding encoding.cpp Iso8859_1.cpp UTF_8.cpp)

target_include_directories (encoding PRIVATE ../TkUtil/public)
target_link_libraries (encoding PUBLIC TkUtil)

# INSTALL_RPATH modifie le rpath pour les libs internes au projet :
set_target_properties (encoding PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1 INSTALL_RPATH ${CMAKE_PACKAGE_RPATH_DIR})

