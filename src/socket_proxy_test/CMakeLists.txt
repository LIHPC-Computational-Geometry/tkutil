include (${CMAKE_SOURCE_DIR}/cmake/version.cmake)
include (${GUIToolkitsVariables_CMAKE_DIR}/common.cmake)
include (${GUIToolkitsVariables_CMAKE_DIR}/workarounds.cmake)

set (ALL_EXECUTABLES echo_clt echo_srv)

foreach (exe ${ALL_EXECUTABLES})
   add_executable (${exe} ${exe}.cpp)
   target_include_directories (${exe} PRIVATE ../TkUtil/public)
   target_link_libraries (${exe} PUBLIC TkUtil)
# INSTALL_RPATH modifie le rpath pour les libs internes au projet :
	set_target_properties (${exe} PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1 INSTALL_RPATH ${CMAKE_PACKAGE_RPATH_DIR})
endforeach (exe)


