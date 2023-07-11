set (CURRENT_PACKAGE_NAME "TkUtilScripting")
set (CURRENT_PACKAGE_MACRO_NAME "TK_UTIL_SCRIPTING")

find_package (GUIToolkitsVariables)

include (${CMAKE_SOURCE_DIR}/cmake/version.cmake)
include (${GUIToolkitsVariables_CMAKE_DIR}/common.cmake)
include (${GUIToolkitsVariables_CMAKE_DIR}/python_binding.cmake)
include (${GUIToolkitsVariables_CMAKE_DIR}/workarounds.cmake)

file (GLOB HEADERS public/${CURRENT_PACKAGE_NAME}/*.h)
file (GLOB CPP_SOURCES *.i *.cpp)

# TkUtilScripting est la cible "binding swig" => donnera à terme _TkUtilScripting.so et TkUtilScripting.py. 
# Dans TkUtilScripting.py on aura import.find_module('_TkUtilScripting', [dirname(__file__)]) ou équivalent (dépend de la version de swig, de python, ...)
# => le nom du module (ici nom de la cible préfixé) est utilisé un peu partout (nom des fichiers, mais également
# par les sources du binding).
# On ne cherche plus à obtenir de libTkUtilScripting, de quelque manière que ce soit :
# - L'objectif est d'utiliser les facilités apportées par CMake (donc find_package (SWIG 3)/SWIG_USE_FILE)
# - Ces facilités fournissent de base _TkUtilScripting (dont le seul regret est de ne pas avoir libTkUtilScripting)
# - Renommer _TkUtilScripting en libTkUtilScripting (via les PROPERTIES SWIG_MODULE_NAME et PREFIX "lib") ne fonctionne pas complétement
# bien. Dans le fichier python généré on a un import_module ("_TkUtilScripting") ou import_module ("libTkUtilScripting")
# selon l'ordre de création des cibles ... (TkUtilScripting.py est créé 2 fois, une pour _TkUtilScripting, une pour libTkUtilScripting)
# - Avoir un lien libTkUtilScripting -> _TkUtilScripting (ou inverse) ne marche pas forcément très bien car le soname
# n'est pas le même que le nom de l'archive, et du coup le fichier recherché au final ne sera pas forcément dans le RPATH ...
# => dans cette version on ne génère que _TkUtilScripting qui sera utilisé directement depuis les scripts comme par les exécutables.
set (ALL_TARGETS TkUtilScripting)
set_property (SOURCE TkUtilScripting.i PROPERTY CPLUSPLUS ON)
swig_add_library (TkUtilScripting TYPE SHARED LANGUAGE PYTHON SOURCES ${CPP_SOURCES})
cmake_policy (SET CMP0086 NEW)
set_source_files_properties (TkUtilScripting.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties (TkUtilScripting.i PROPERTIES SWIG_MODULE_NAME TkUtil)
add_custom_command (TARGET TkUtilScripting POST_BUILD COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/TkUtil.py ${CMAKE_CURRENT_BINARY_DIR}/binding.py COMMAND cat ${GUIToolkitsVariables_CMAKE_DIR}/utf8-header.py ${CMAKE_CURRENT_BINARY_DIR}/binding.py >> ${CMAKE_CURRENT_BINARY_DIR}/TkUtilScripting.py)
# Objectif avoir les flags de compilation SWIG suivants :
# -globals TkUtil -shadow -DIN_STD="std::" -DSWIG_FORBIDS_FINALIZATION -DIN_UTIL="TkUtil::" -DIN_STD="std::"
# Ne marche pas sans le -c++ malgré les CPLUSPLUS ON avant le swig_add_library.
#set (MT_SWIG_COMPILE_OPTIONS "-c++;-module;TkUtil;-globals;TkUtil;-shadow;-DIN_STD=\"std::\";-DSWIG_FORBIDS_FINALIZATION;-DIN_UTIL=\"TkUtil::\";-DIN_STD=\"std::\"")
set (MT_SWIG_COMPILE_OPTIONS "-c++;-module;TkUtil;-globals;TkUtil;-shadow;-DIN_STD=\"std::\";-DSWIG_FORBIDS_FINALIZATION;-DIN_UTIL=\"TkUtil::\";-DIN_STD=\"std::\"")
if (SWIG_VERSION VERSION_GREATER 4)
	list (APPEND MT_SWIG_COMPILE_OPTIONS "-doxygen")
endif (SWIG_VERSION VERSION_GREATER 4)
# Avec cmake_policy (SET CMP0086 NEW) -module ${SWIG_MODULE_NAME} devrait être automatiquement passé, SWIG_MODULE_NAME étant défini comme propriété du fichier swig. Mais ...
swig_link_libraries (TkUtilScripting PUBLIC TkUtil ${Python_LIBRARIES})
target_include_directories (TkUtilScripting PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)
foreach (lib ${ALL_TARGETS})
   target_include_directories (TkUtilScripting PRIVATE ${Python_INCLUDE_DIRS})
   target_compile_options (${lib} PUBLIC ${MANDATORY_CXX_OPTIONS})
   set_property (TARGET ${lib} PROPERTY VERSION ${TK_UTIL_VERSION})
   set_property (TARGET ${lib} PROPERTY SOVERSION ${TK_UTIL_MAJOR_VERSION})
   set_property (TARGET ${lib} PROPERTY SWIG_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src/TkUtil/public)
   set_property (TARGET ${lib} PROPERTY SWIG_COMPILE_OPTIONS ${MT_SWIG_COMPILE_OPTIONS})
# INSTALL_RPATH modifie le rpath pour les libs internes au projet :
# Etre capable une fois installée de retrouver les libs utilisées (iconv ?), ... :
message ("INSTALL RPATH=" "${CMAKE_PACKAGE_RPATH_DIR};${CMAKE_PYTHON_RPATH_DIR}")
message ("INSTALL RPATH=" "${CMAKE_PACKAGE_RPATH_DIR};${CMAKE_PYTHON_RPATH_DIR};$ORIGIN/../${PYTHON_BINDING_DIR}")
#   set_target_properties (${lib} PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1 INSTALL_RPATH "${CMAKE_PACKAGE_RPATH_DIR};${CMAKE_PYTHON_RPATH_DIR}")
   set_target_properties (${lib} PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1 INSTALL_RPATH "${CMAKE_PACKAGE_RPATH_DIR};$ORIGIN/../${PYTHON_BINDING_DIR}")
endforeach (lib)

# INSTALLATION :
include(CMakePackageConfigHelpers)
# Génération du fichier contenant les informations sur la distribution :
configure_file (cmake/TkUtilScriptingDistribution.h.in  ${CMAKE_CURRENT_BINARY_DIR}/TkUtilScriptingDistribution.tmp)
configure_file (${CMAKE_CURRENT_BINARY_DIR}/TkUtilScriptingDistribution.tmp  ${CMAKE_CURRENT_BINARY_DIR}/TkUtilScriptingDistribution.h)

# ConfigPackageLocation : c'est plus ou moins standardisé, le défaut étant lib/cmake.
# On nos recommande très vivement d'utiliser ce répertoire afin de limiter les soucis par la suite,
# notamment au niveau des vues.
set (ConfigPackageLocation ${CMAKE_CMAKE_DIR})

# Création du fichier TkUtilScriptingConfig.cmake qui définira TkUtilScripting_INCLUDE_DIR, TkUtilScripting_LIB_DIR et TkUtilScripting_RUNTIME_DIR (_TkUtilScripting.so et .py) :
# On fait ici une installation "inhabituelle". S'agissant d'un binding python il est installé dans /install_prefix/lib/python/site-packages
# Mais comme on en veut une lib également (_TkUtilScripting.so), pour les logiciels, on installe également la version bibliothèque (via un lien) dans le répertoire standard.
# Installation de la bibliothèque :
install (TARGETS TkUtilScripting EXPORT ${CMAKE_PACKAGE_TARGETS_NAME} 
         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
         INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Recopie des en-têtes dans le répertoire d'installation :
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CURRENT_PACKAGE_NAME})
# Dont TkUtilScriptingDistribution.h généré plus haut :
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/TkUtilScriptingDistribution.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CURRENT_PACKAGE_NAME})

# Installation du binding python :
# On ajoute en tête du fichier généré par swig l'encoding utilisé car sinon ça ne passe pas forcément lors de son exécution. Pour ce on le renomme en binding.py, puis on
# fait un cat entete.py binding.py dont la sortie standard devient TkUtilScripting.py que l'on installera alors.
install (CODE "file (RENAME ${CMAKE_CURRENT_BINARY_DIR}/TkUtilScripting.py ${CMAKE_CURRENT_BINARY_DIR}/binding.py)")
install (CODE "execute_process (COMMAND cat ${GUIToolkitsVariables_CMAKE_DIR}/utf8-header.py ${CMAKE_CURRENT_BINARY_DIR}/binding.py OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/TkUtilScripting.py)")
# Recopie du fichier binding python (=> création de PYTHON_BINDING_DIR utilisé après par install) :
install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/TkUtilScripting.py DESTINATION ${PYTHON_BINDING_DIR})
# Le lien lib/_TkUtilScripting.so.x.y.z vers lib/python/site-packages/_TkUtilScripting.so.x.y.z n'est pas bon car ils ne sont plus dans le même répertoire ...
install (CODE "execute_process (COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/_TkUtilScripting.so.${TK_UTIL_VERSION} ${CMAKE_INSTALL_PREFIX}/${PYTHON_BINDING_DIR}/_TkUtilScripting.so.${TK_UTIL_VERSION})")
install (CODE "execute_process (COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/${PYTHON_BINDING_DIR}/_TkUtilScripting.so.${TK_UTIL_VERSION} ${CMAKE_INSTALL_PREFIX}/${PYTHON_BINDING_DIR}/_TkUtilScripting.so.${TK_UTIL_MAJOR_VERSION})")
install (CODE "execute_process (COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/${PYTHON_BINDING_DIR}/_TkUtilScripting.so.${TK_UTIL_MAJOR_VERSION} ${CMAKE_INSTALL_PREFIX}/${PYTHON_BINDING_DIR}/_TkUtilScripting.so)")

# Le fichier contenant les informations de version de ce module :
write_basic_package_version_file (${CMAKE_PACKAGE_VERSION_FILE} VERSION ${TK_UTIL_VERSION} COMPATIBILITY SameMajorVersion)
export (EXPORT ${CMAKE_PACKAGE_TARGETS_NAME} FILE ${CMAKE_PACKAGE_TARGETS_FILE} NAMESPACE ${CMAKE_MODULE_NAMESPACE})

# Utilisation par d'autres modules de TkUtilScripting :
set (INCLUDE_INSTALL_DIR include/)			# TkUtilScripting_INCLUDE_DIR avec TkUtilScriptingConfig.cmake.in
set (LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})		# TkUtilScripting_LIB_DIR avec TkUtilScriptingConfig.cmake.in
set (RUNTIME_INSTALL_DIR ${PYTHON_BINDING_DIR}/)	# TkUtilScripting_RUNTIME_DIR avec TkUtilScriptingConfig.cmake.in
# Création du fichier TkUtilScriptingConfig.cmake qui définira TkUtilScripting_INCLUDE_DIR, TkUtilScripting_LIB_DIR et TkUtilScripting_RUNTIME_DIR :
configure_package_config_file (cmake/${CMAKE_PACKAGE_CONFIG_NAME}.cmake.in ${CMAKE_PACKAGE_CONFIG_FILE}
	INSTALL_DESTINATION ${CMAKE_CMAKE_DIR}
	PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR RUNTIME_INSTALL_DIR)

# Copie du fichier TkUtilScriptingTargets.cmake dans prefix/lib/TkUtilScripting/cmake :
install (EXPORT ${CMAKE_PACKAGE_TARGETS_NAME}
	FILE ${CMAKE_PACKAGE_TARGETS_BASENAME} NAMESPACE ${CMAKE_MODULE_NAMESPACE}
	DESTINATION ${ConfigPackageLocation} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
	)

# Copie des fichiers TkUtilScriptingConfig.cmake et TkUtilScriptingConfigVersion.cmake dans prefix/lib/TkUtilScripting/cmake :
install (
   FILES ${CMAKE_PACKAGE_CONFIG_FILE} ${CMAKE_PACKAGE_VERSION_FILE}
   DESTINATION ${ConfigPackageLocation} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
   )

# La documentation doxygen de l'API LemKernelScripting :
find_package (Doxygen)
if (Doxygen_FOUND)
	set (DOXYGEN_SOURCES ${HEADERS})
	set (DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_DOCDIR}/TkUtilScripting)
	set (DOXYGEN_PROJECT_NUMBER ${TK_UTIL_VERSION})
	set (DOXYGEN_PROJECT_BRIEF "API Python de la bibliothèque TkUtil")
	doxygen_add_docs (TkUtilScriptingHTMLDoc ${DOXYGEN_SOURCES} ALL)
	install (
		DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_DOCDIR}/TkUtilScripting" DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETGID DESTINATION "${CMAKE_INSTALL_FULL_DOCDIR}"
		)
else (Doxygen_FOUND)
	message (WARNING "Documentation de l'API python non générée : doxygen non trouvé.")
endif (Doxygen_FOUND)
