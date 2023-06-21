include (${CMAKE_SOURCE_DIR}/cmake/version.cmake)
include (${GUIToolkitsVariables_CMAKE_DIR}/common.cmake)
include (${GUIToolkitsVariables_CMAKE_DIR}/workarounds.cmake)

set (ALL_EXECUTABLES
   ansi_esc_codes canceled_threads conversions date exceptions
   fileExtractor fileinfos fileopts hostinfos joinable locale
   logs memory modify_script remoteProcess removefile script_tags
   scripting_logs thread_manager thread_pool timer tmpfile
   unicode urlfifo user_representation userinfos utf8 utf8chars
   )

enable_testing()

foreach (exe ${ALL_EXECUTABLES})
   add_executable (${exe} ${exe}.cpp)
   target_include_directories (${exe} PRIVATE ../TkUtil/public)
   target_link_libraries (${exe} PUBLIC TkUtil)
# INSTALL_RPATH modifie le rpath pour les libs internes au projet :
	set_target_properties (${exe} PROPERTIES 
		INSTALL_RPATH_USE_LINK_PATH 1
		INSTALL_RPATH ${CMAKE_PACKAGE_RPATH_DIR})
endforeach (exe)


### Tests not added to make test
#
# fileExtractor, modify_script and script_tags: 
# they are very specific and can not be executed in CI.
#
# locale: minimal container environment used in CI as no locale except UTF8
#
# remoteProcess: tests the client/server communication and lifecycle (remote kill)
# unused in CI

add_test(NAME ansi_esc_codes COMMAND $<TARGET_FILE:ansi_esc_codes>)
add_test(NAME canceled_threads COMMAND $<TARGET_FILE:canceled_threads>)
add_test(NAME conversions COMMAND $<TARGET_FILE:conversions>)
add_test(NAME date COMMAND $<TARGET_FILE:date>)
add_test(NAME exceptions COMMAND $<TARGET_FILE:exceptions>)
add_test(NAME fileinfos COMMAND $<TARGET_FILE:fileinfos> /etc/os-release)
add_test(NAME fileopts COMMAND $<TARGET_FILE:fileopts>)
add_test(NAME hostinfos COMMAND $<TARGET_FILE:hostinfos>)
add_test(NAME joinable COMMAND $<TARGET_FILE:joinable>)
add_test(NAME logs COMMAND $<TARGET_FILE:logs>)
add_test(NAME memory COMMAND $<TARGET_FILE:memory>)
execute_process (COMMAND bash -c "touch /tmp/my_empty_file")
add_test(NAME removefile COMMAND $<TARGET_FILE:removefile> /tmp/my_empty_file)
add_test(NAME scripting_logs COMMAND $<TARGET_FILE:scripting_logs>)
add_test(NAME thread_manager COMMAND $<TARGET_FILE:thread_manager>)
add_test(NAME thread_pool COMMAND $<TARGET_FILE:thread_pool>)
add_test(NAME timer COMMAND $<TARGET_FILE:timer>)
add_test(NAME tmpfile COMMAND $<TARGET_FILE:tmpfile> my_dir_prefix my_file_prefix -r)
add_test(NAME unicode COMMAND $<TARGET_FILE:unicode>)
add_test(NAME urlfifo COMMAND $<TARGET_FILE:urlfifo>)
add_test(NAME user_representation COMMAND $<TARGET_FILE:user_representation> 31415169e-4)
add_test(NAME userinfos COMMAND $<TARGET_FILE:userinfos>)
add_test(NAME utf8 COMMAND $<TARGET_FILE:utf8>)
add_test(NAME utf8chars COMMAND $<TARGET_FILE:utf8chars>)
