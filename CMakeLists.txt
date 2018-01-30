cmake_minimum_required (VERSION 3.7)
project (ShipSoftwareBackend C)

# Version number
set(ShipSoftwareBackend_VERSION_MAJOR 1)
set(ShipSoftwareBackend_VERSION_MINOR 0)
set(ShipSoftwareBackend_VERSION_PATCH 0)
set(ShipSoftwareBackend_VERSION ${ShipSoftwareBackend_VERSION_MAJOR}.${ShipSoftwareBackend_VERSION_MINOR}.${ShipSoftwareBackend_VERSION_PATCH})
configure_file(
    "${PROJECT_SOURCE_DIR}/src/version.h.in"
    "${PROJECT_BINARY_DIR}/src/version.h"
    @ONLY
)

include_directories("${PROJECT_BINARY_DIR}/src")

find_package(PkgConfig REQUIRED)

# MariaDB
pkg_check_modules(MARIADB REQUIRED mariadb)
include_directories(${MARIADB_INCLUDE_DIRS})
link_directories(${MARIADB_LIBRARY_DIRS})
add_definitions(${MARIADB_CFLAGS_OTHER})
list(APPEND SOURCES "src/api_thread.c" "src/database.c")

# json-glib
pkg_check_modules(JSON REQUIRED json-glib-1.0)
include_directories(${JSON_INCLUDE_DIRS})
link_directories(${JSON_LIBRARY_DIRS})
add_definitions(${JSON_CFLAGS_OTHER})
list(APPEND SOURCES "src/config.c" "src/json.c")

# cURL
pkg_check_modules(CURL REQUIRED libcurl)
include_directories(${CURL_INCLUDE_DIRS})
link_directories(${CURL_LIBRARY_DIRS})
add_definitions(${CURL_CFLAGS_OTHER})
list(APPEND SOURCES "src/api.c")

# GTK
option(WITH_GUI "Build with GTK+ GUI" ON)
if (WITH_GUI)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    include_directories(${GTK3_INCLUDE_DIRS})
    link_directories(${GTK3_LIBRARY_DIRS})
    list(APPEND SOURCES "src/main_window.c" "src/config_window.c" "src/dialogs.c")
    add_definitions(-DWITH_GUI ${GTK3_CFLAGS_OTHER})
endif()

# Documentation
option(BUILD_DOC "Build documentation" OFF)
if (BUILD_DOC)
    find_package(Doxygen REQUIRED)
    if (NOT DOXYGEN_DOT_FOUND)
        message(FATAL_ERROR "Generation of Doxygen documentation requires `dot` utility!")
    endif ()
    set(DOXYGENCONF_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen.conf.in)
    set(DOXYGENCONF ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)

    configure_file(${DOXYGENCONF_IN} ${DOXYGENCONF} @ONLY)

    add_custom_target(doc ALL COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGENCONF}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Generating documentation with Doxygen" VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()

if (DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
endif()

list(APPEND SOURCES "src/ship_defines.h")

if (NOT WIN32)
    add_executable(shipsoftware_backend src/main.c ${SOURCES})
else()
    add_executable(shipsoftware_backend WIN32 src/main.c ${SOURCES})
endif()
target_link_libraries(shipsoftware_backend m ${MARIADB_LIBRARIES} ${ODBC_LIBRARIES} ${GTK3_LIBRARIES} ${JSON_LIBRARIES} ${CURL_LIBRARIES})

if (WIN32)
    add_custom_command(TARGET shipsoftware_backend POST_BUILD COMMAND ${PROJECT_SOURCE_DIR}/scripts/mingw-bundledlls ${PROJECT_BINARY_DIR}/shipsoftware_backend.exe --copy)
    add_custom_command(TARGET shipsoftware_backend POST_BUILD COMMAND ${PROJECT_SOURCE_DIR}/scripts/mingw-gtktheme)
endif()

