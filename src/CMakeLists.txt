if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(CheckLibraryExists)
include(ExternalProject)
include(FindPkgConfig)

add_compile_options(-Wall -Wextra -pedantic)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_LINK_WHAT_YOU_USE OFF)
set(CMAKE_SKIP_BUILD_RPATH ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

option(BUILD_STATIC_BINARY      "Build static binary"         ON)
option(FORCE_EXTERNAL_LIBEV     "Do not use system libev"     OFF)
option(FORCE_EXTERNAL_LIBADA    "Do not use system libada"    OFF)
option(FORCE_EXTERNAL_LIBLLHTTP "Do not use system libllhttp" OFF)
option(FORCE_EXTERNAL_SQLITE3   "Do not use system sqlite3"   OFF)

if(BUILD_STATIC_BINARY)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

find_program(CCACHE_EXE ccache)
if(CCACHE_EXE)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_EXE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_EXE})
endif()

add_executable(
    tfhttp
    main.cpp
    clienthandler.cpp
    clienthandler_p.cpp
    database.cpp
    database_p.cpp
    server.cpp
    server_p.cpp
    serversocket.cpp
    serversocket_p.cpp
)

target_precompile_headers(tfhttp PRIVATE stdafx.h)

if(NOT FORCE_EXTERNAL_LIBEV)
    pkg_check_modules(EV libev)
    if(NOT EV_FOUND)
        find_path(
            EV_INCLUDE_DIRS
            NAMES ev.h
        )

        find_library(
            EV_LIBRARIES
            NAMES ev
        )

        if(EV_INCLUDE_DIRS AND EV_LIBRARIES)
            set(EV_FOUND TRUE)
        else()
            set(EV_FOUND FALSE)
        endif()
    endif()
endif()

if(NOT FORCE_EXTERNAL_LIBADA)
    find_path(
        ADA_INCLUDE_DIRS
        NAMES ada.h
    )

    find_library(
        ADA_LIBRARIES
        NAMES ada
    )

    if(ADA_INCLUDE_DIRS AND ADA_LIBRARIES)
        set(ADA_FOUND TRUE)
    else()
        set(ADA_FOUND FALSE)
    endif()
endif()

if(NOT FORCE_EXTERNAL_LIBLLHTTP)
    pkg_check_modules(LLHTTP libllhttp)
endif()

if(NOT FORCE_EXTERNAL_SQLITE3)
    pkg_check_modules(SQLITE3 sqlite3)
endif()

set(MATH_LIBS "")
check_library_exists(m floor "" LIBM)
if(LIBM)
    list(APPEND MATH_LIBS "m")
endif()

find_path(SQLITE3PP sqlite3pp.h)
find_path(NLOHMANN_JSON nlohmann/json.hpp)

if(NOT EV_FOUND)
    ExternalProject_Add(
        libev
        URL http://dist.schmorp.de/libev/Attic/libev-4.33.tar.gz
        URL_HASH MD5=a3433f23583167081bf4acdd5b01b34f
        DOWNLOAD_NO_PROGRESS ON
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --quiet --enable-silent-rules --disable-dependency-tracking --prefix=${CMAKE_CURRENT_BINARY_DIR}/libev CC=${CMAKE_C_COMPILER} AR=${CMAKE_AR} RANLIB=${CMAKE_RANLIB}
    )

    set(EV_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/libev/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ev${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(EV_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/libev/include")
    add_dependencies(tfhttp libev)
endif()

if(NOT ADA_FOUND)
    ExternalProject_Add(
        libada
        GIT_REPOSITORY https://github.com/ada-url/ada.git
        GIT_TAG v2.7.4
        GIT_SHALLOW ON
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=MinSizeRel -DADA_TOOLS=off -DADA_TESTING=off -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libada
    )

    set(ADA_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/libada/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ada${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(ADA_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/libada/include")
    add_dependencies(tfhttp libada)
endif()

if(NOT LLHTTP_FOUND)
    ExternalProject_Add(
        libllhttp
        URL https://github.com/nodejs/llhttp/archive/refs/tags/release/v9.1.3.tar.gz
        URL_HASH MD5=1e87982dab458e5a26259fe36c5c55e7
        DOWNLOAD_NO_PROGRESS ON
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_STATIC_LIBS=on -DBUILD_SHARED_LIBS=off -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libllhttp
    )

    set(LLHTTP_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/libllhttp/lib/${CMAKE_STATIC_LIBRARY_PREFIX}llhttp${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(LLHTTP_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/libllhttp/include")
    add_dependencies(tfhttp libllhttp)
endif()

if(NOT SQLITE3_FOUND)
    set(SQLITE3_CFLAGS "-DSQLITE_DQS=0 -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_DEFAULT_WAL_SYNCHRONOUS=1 -DSQLITE_MAX_EXPR_DEPTH=0 -DSQLITE_OMIT_DECLTYPE -DSQLITE_OMIT_DEPRECATED -DSQLITE_OMIT_PROGRESS_CALLBACK -DSQLITE_OMIT_SHARED_CACHE -DSQLITE_DISABLE_FTS3_UNICODE -DSQLITE_DISABLE_FTS4_DEFERRED")
    ExternalProject_Add(
        sqlite3
        GIT_REPOSITORY https://github.com/sqlite/sqlite.git
        GIT_TAG version-3.44.2
        GIT_SHALLOW ON
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --quiet -disable-threadsafe --enable-releasemode --disable-readline --disable-tcl --enable-static --disable-shared --enable-tempstore=yes --disable-load-extension --disable-math --disable-json --prefix=${CMAKE_CURRENT_BINARY_DIR}/sqlite3 CC=${CMAKE_C_COMPILER} CPPFLAGS=${SQLITE3_CFLAGS}
    )

    set(SQLITE3_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/sqlite3/lib/${CMAKE_STATIC_LIBRARY_PREFIX}sqlite3${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(SQLITE3_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/sqlite3/include")
    add_dependencies(tfhttp sqlite3)
endif()

if(NOT SQLITE3PP)
    ExternalProject_Add(
        sqlite3pp
        GIT_REPOSITORY https://github.com/iwongu/sqlite3pp.git
        GIT_TAG v1.0.9
        GIT_SHALLOW ON
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
    )

    target_include_directories(tfhttp PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/sqlite3pp-prefix/src/sqlite3pp/headeronly_src)
    add_dependencies(tfhttp sqlite3pp)
else()
    target_include_directories(tfhttp PRIVATE ${SQLITE3PP})
endif()

if(NOT NLOHMANN_JSON)
    ExternalProject_Add(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW ON
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=MinSizeRel -DJSON_BuildTests=off -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/json
    )

    target_include_directories(tfhttp PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/json/include)
    add_dependencies(tfhttp nlohmann_json)
else()
    target_include_directories(tfhttp PRIVATE ${NLOHMANN_JSON})
endif()

target_include_directories(tfhttp PRIVATE ${EV_INCLUDE_DIRS})
target_link_directories(tfhttp PRIVATE ${EV_LIBRARY_DIRS})
target_link_libraries(tfhttp PRIVATE ${EV_LIBRARIES} ${MATH_LIBS})

target_include_directories(tfhttp PRIVATE ${LLHTTP_INCLUDE_DIRS})
target_link_directories(tfhttp PRIVATE ${LLHTTP_LIBRARY_DIRS})
target_link_libraries(tfhttp PRIVATE ${LLHTTP_LIBRARIES})

target_include_directories(tfhttp PRIVATE ${ADA_INCLUDE_DIRS})
target_link_directories(tfhttp PRIVATE ${ADA_LIBRARY_DIRS})
target_link_libraries(tfhttp PRIVATE ${ADA_LIBRARIES})

target_include_directories(tfhttp PRIVATE ${SQLITE3_INCLUDE_DIRS})
target_link_directories(tfhttp PRIVATE ${SQLITE3_LIBRARY_DIRS})
target_link_libraries(tfhttp PRIVATE ${SQLITE3_LIBRARIES})
