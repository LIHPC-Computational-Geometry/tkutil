include (${CMAKE_SOURCE_DIR}/cmake/version.cmake)
include (${CMAKE_SOURCE_DIR}/cmake/common.cmake)
include (${CMAKE_SOURCE_DIR}/cmake/workarounds.cmake)

add_executable (socket_proxy socket_proxy.cpp)

target_include_directories (socket_proxy PRIVATE ../TkUtil/public)
target_link_libraries (socket_proxy PUBLIC TkUtil)

# INSTALL_RPATH modifie le rpath pour les libs internes au projet :
set_target_properties (socket_proxy PROPERTIES 
		INSTALL_RPATH_USE_LINK_PATH 1
		INSTALL_RPATH ${CMAKE_PACKAGE_RPATH_DIR})

