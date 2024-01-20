if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(CheckLibraryExists)
include(ExternalProject)
include(FindPkgConfig)

add_compile_options(-Wall -Wextra -pedantic)
# add_compile_options(-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-pre-c++17-compat -Wno-c++20-compat -Wno-clang-diagnostic-unused-macros)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_LINK_WHAT_YOU_USE OFF)
set(CMAKE_SKIP_BUILD_RPATH ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

option(BUILD_STATIC_BINARY        "Build static binary"         OFF)
option(BUILD_MOSTLY_STATIC_BINARY "All dependencies except system libraries are static" ON)
option(FORCE_EXTERNAL_LIBEV       "Do not use system libev"     OFF)
option(FORCE_EXTERNAL_LIBADA      "Do not use system libada"    OFF)
option(FORCE_EXTERNAL_LIBLLHTTP   "Do not use system libllhttp" OFF)
option(FORCE_EXTERNAL_SQLITE3     "Do not use system sqlite3"   OFF)
option(FORCE_EXTERNAL_LIBTLS      "Do not use system libtls"    OFF)

if(BUILD_STATIC_BINARY)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(CMAKE_EXE_LINKER_FLAGS "-static")
    set(BUILD_MOSTLY_STATIC_BINARY OFF)
elseif(BUILD_MOSTLY_STATIC_BINARY)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
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
    tlsconfigurator.cpp
    tlsconfigurator_p.cpp
    tlsexception.cpp
    tlsservercontext.cpp
    tlsservercontext_p.cpp
    tlsutils.cpp
)

target_precompile_headers(tfhttp PRIVATE stdafx.h)

if(NOT FORCE_EXTERNAL_LIBEV)
    find_package(libev)
endif()

if(NOT FORCE_EXTERNAL_LIBADA)
    find_package(libada)
endif()

if(NOT FORCE_EXTERNAL_LIBLLHTTP)
    find_package(libllhttp)
endif()

if(NOT FORCE_EXTERNAL_SQLITE3)
    find_package(libsqlite3)
endif()

if(NOT FORCE_EXTERNAL_LIBTLS)
    find_package(libtls)
endif()

if(BUILD_MOSTLY_STATIC_BINARY)
    if (LIBEV_FOUND AND NOT LIBEV_STATIC_LIBRARIES)
        message(WARNING "static version of libev not found")
        unset(LIBEV_FOUND)
    endif()

    if (LIBADA_FOUND AND NOT LIBADA_STATIC_LIBRARIES)
        message(WARNING "static version of libada not found")
        unset(LIBADA_FOUND)
    endif()

    if (LIBLLHTTP_FOUND AND NOT LIBLLHTTP_STATIC_LIBRARIES)
        message(WARNING "static version of libllhttp not found")
        unset(LIBLLHTTP_FOUND)
    endif()

    if (LIBSQLITE3_FOUND AND NOT LIBSQLITE3_STATIC_LIBRARIES)
        message(WARNING "static version of libsqlite3 not found")
        unset(LIBSQLITE3_FOUND)
    endif()
endif()

find_path(SQLITE3PP sqlite3pp.h)
find_path(NLOHMANN_JSON nlohmann/json.hpp)

if(NOT LIBEV_FOUND)
    set(LIBEV_CPPFLAGS -DEV_COMPAT3=0 -DEV_PREPARE_ENABLE=0 -DEV_CHECK_ENABLE=0 -DEV_IDLE_ENABLE=0 -DEV_FORK_ENABLE=0 -DEV_CLEANUP_ENABLE=0 -DEV_CHILD_ENABLE=0 -DEV_ASYNC_ENABLE=0 -DEV_EMBED_ENABLE=0)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND LIBEV_CPPFLAGS -DEV_VERIFY=3)
        set(LIBEV_CFLAGS -Og -g3)
    else()
        set(LIBEV_CFLAGS -O3 -g)
    endif()
    list(JOIN LIBEV_CPPFLAGS " " LIBEV_CPPFLAGS)
    list(JOIN LIBEV_CFLAGS " " LIBEV_CFLAGS)

    ExternalProject_Add(
        libev
        URL http://dist.schmorp.de/libev/Attic/libev-4.33.tar.gz
        URL_HASH MD5=a3433f23583167081bf4acdd5b01b34f
        DOWNLOAD_NO_PROGRESS ON
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --quiet --enable-static --disable-shared --prefix=${CMAKE_CURRENT_BINARY_DIR}/libev CC=${CMAKE_C_COMPILER} AR=${CMAKE_AR} RANLIB=${CMAKE_RANLIB} CPPFLAGS=${LIBEV_CPPFLAGS} CFLAGS=${LIBEV_CFLAGS}
    )

    unset(LIBEV_CPPFLAGS)
    unset(LIBEV_CFLAGS)
    set(LIBEV_LIBRARIES ev;m)
    set(LIBEV_LIBRARY_DIRS "${CMAKE_CURRENT_BINARY_DIR}/libev/lib")
    set(LIBEV_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/libev/include")

    set(LIBEV_STATIC_LIBRARIES ${LIBEV_LIBRARIES})
    set(LIBEV_STATIC_LIBRARY_DIRS ${LIBEV_LIBRARY_DIRS})
    set(LIBEV_STATIC_INCLUDE_DIRS ${LIBEV_INCLUDE_DIRS})

    add_dependencies(tfhttp libev)
endif()

if(NOT LIBADA_FOUND)
    ExternalProject_Add(
        libada
        GIT_REPOSITORY https://github.com/ada-url/ada.git
        GIT_TAG v2.7.4
        GIT_SHALLOW ON
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_SHARED_LIBS=OFF -DADA_TOOLS=off -DADA_TESTING=off -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libada
    )

    set(LIBADA_LIBRARIES ada)
    set(LIBADA_LIBRARY_DIRS "${CMAKE_CURRENT_BINARY_DIR}/libada/lib")
    set(LIBADA_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/libada/include")

    set(LIBADA_STATIC_LIBRARIES ${LIBADA_LIBRARIES})
    set(LIBADA_STATIC_LIBRARY_DIRS ${LIBADA_LIBRARY_DIRS})
    set(LIBADA_STATIC_INCLUDE_DIRS ${LIBADA_INCLUDE_DIRS})

    add_dependencies(tfhttp libada)
endif()

if(NOT LIBLLHTTP_FOUND)
    ExternalProject_Add(
        libllhttp
        URL https://github.com/nodejs/llhttp/archive/refs/tags/release/v9.1.3.tar.gz
        URL_HASH MD5=1e87982dab458e5a26259fe36c5c55e7
        DOWNLOAD_NO_PROGRESS ON
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libllhttp
    )

    set(LIBLLHTTP_LIBRARIES llhttp)
    set(LIBLLHTTP_LIBRARY_DIRS "${CMAKE_CURRENT_BINARY_DIR}/libllhttp/lib")
    set(LIBLLHTTP_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/libllhttp/include")

    set(LIBLLHTTP_STATIC_LIBRARIES ${LIBLLHTTP_LIBRARIES})
    set(LIBLLHTTP_STATIC_LIBRARY_DIRS ${LIBLLHTTP_LIBRARY_DIRS})
    set(LIBLLHTTP_STATIC_INCLUDE_DIRS ${LIBLLHTTP_INCLUDE_DIRS})

    add_dependencies(tfhttp libllhttp)
endif()

if(NOT LIBSQLITE3_FOUND)
    set(SQLITE3_CPPFLAGS "-DSQLITE_DQS=0 -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_DEFAULT_WAL_SYNCHRONOUS=1 -DSQLITE_MAX_EXPR_DEPTH=0 -DSQLITE_OMIT_DECLTYPE -DSQLITE_OMIT_DEPRECATED -DSQLITE_OMIT_PROGRESS_CALLBACK -DSQLITE_OMIT_SHARED_CACHE -DSQLITE_DISABLE_FTS3_UNICODE -DSQLITE_DISABLE_FTS4_DEFERRED")
    ExternalProject_Add(
        libsqlite3
        GIT_REPOSITORY https://github.com/sqlite/sqlite.git
        GIT_TAG version-3.44.2
        GIT_SHALLOW ON
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --quiet -disable-threadsafe --enable-releasemode --disable-readline --disable-amalgamation --disable-tcl --enable-static --disable-shared --enable-tempstore=yes --disable-load-extension --disable-math --disable-json --without-tcl --prefix=${CMAKE_CURRENT_BINARY_DIR}/sqlite3 CC=${CMAKE_C_COMPILER} CPPFLAGS=${SQLITE3_CPPFLAGS}
    )

    set(LIBSQLITE3_LIBRARIES sqlite3)
    set(LIBSQLITE3_LIBRARY_DIRS "${CMAKE_CURRENT_BINARY_DIR}/sqlite3/lib")
    set(LIBSQLITE3_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/sqlite3/include")

    set(LIBSQLITE3_STATIC_LIBRARIES ${LIBSQLITE3_LIBRARIES})
    set(LIBSQLITE3_STATIC_LIBRARY_DIRS ${LIBSQLITE3_LIBRARY_DIRS})
    set(LIBSQLITE3_STATIC_INCLUDE_DIRS ${LIBSQLITE3_INCLUDE_DIRS})

    add_dependencies(tfhttp libsqlite3)
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

if(NOT LIBTLS_FOUND)
    ExternalProject_Add(
        libtls
        GIT_REPOSITORY https://git.causal.agency/libretls
        GIT_TAG 3.8.1
        GIT_SHALLOW ON
        CONFIGURE_COMMAND autoreconf -i <SOURCE_DIR> && <SOURCE_DIR>/configure --disable-dependency-tracking --disable-shared --enable-static --prefix=${CMAKE_CURRENT_BINARY_DIR}/libtls CC=${CMAKE_C_COMPILER} AR=${CMAKE_AR} RANLIB=${CMAKE_RANLIB}
    )

    set(LIBTLS_LIBRARIES tls;ssl;crypto)
    set(LIBTLS_LIBRARY_DIRS "${CMAKE_CURRENT_BINARY_DIR}/libtls/lib")
    set(LIBTLS_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/libtls/include")

    set(LIBTLS_STATIC_LIBRARIES ${LIBTLS_LIBRARIES})
    set(LIBTLS_STATIC_LIBRARY_DIRS ${LIBTLS_LIBRARY_DIRS})
    set(LIBTLS_STATIC_INCLUDE_DIRS ${LIBTLS_INCLUDE_DIRS})

    add_dependencies(tfhttp libtls)
endif()

if(BUILD_STATIC_BINARY OR BUILD_MOSTLY_STATIC_BINARY)
    target_include_directories(tfhttp PRIVATE ${LIBEV_STATIC_INCLUDE_DIRS})
    target_link_directories(tfhttp PRIVATE ${LIBEV_STATIC_LIBRARY_DIRS})
    if(BUILD_MOSTLY_STATIC_BINARY)
        target_link_libraries(tfhttp PRIVATE -Wl,-Bstatic;${LIBEV_STATIC_LIBRARIES};-Wl,-Bdynamic)
    else()
        target_link_libraries(tfhttp PRIVATE ${LIBEV_STATIC_LIBRARIES})
    endif()
else()
    target_include_directories(tfhttp PRIVATE ${LIBEV_INCLUDE_DIRS})
    target_link_directories(tfhttp PRIVATE ${LIBEV_LIBRARY_DIRS})
    target_link_libraries(tfhttp PRIVATE ${LIBEV_LIBRARIES})
endif()

if(BUILD_STATIC_BINARY OR BUILD_MOSTLY_STATIC_BINARY)
    target_include_directories(tfhttp PRIVATE ${LIBLLHTTP_STATIC_INCLUDE_DIRS})
    target_link_directories(tfhttp PRIVATE ${LIBLLHTTP_STATIC_LIBRARY_DIRS})
    if(BUILD_MOSTLY_STATIC_BINARY)
        target_link_libraries(tfhttp PRIVATE -Wl,-Bstatic;${LIBLLHTTP_STATIC_LIBRARIES};-Wl,-Bdynamic)
    else()
        target_link_libraries(tfhttp PRIVATE ${LIBLLHTTP_STATIC_LIBRARIES})
    endif()
else()
    target_include_directories(tfhttp PRIVATE ${LIBLLHTTP_INCLUDE_DIRS})
    target_link_directories(tfhttp PRIVATE ${LIBLLHTTP_LIBRARY_DIRS})
    target_link_libraries(tfhttp PRIVATE ${LIBLLHTTP_LIBRARIES})
endif()

if(BUILD_STATIC_BINARY OR BUILD_MOSTLY_STATIC_BINARY)
    target_include_directories(tfhttp PRIVATE ${LIBADA_STATIC_INCLUDE_DIRS})
    target_link_directories(tfhttp PRIVATE ${LIBADA_STATIC_LIBRARY_DIRS})
    if(BUILD_MOSTLY_STATIC_BINARY)
        target_link_libraries(tfhttp PRIVATE -Wl,-Bstatic;${LIBADA_STATIC_LIBRARIES};-Wl,-Bdynamic)
    else()
        target_link_libraries(tfhttp PRIVATE ${LIBADA_STATIC_LIBRARIES})
    endif()
else()
    target_include_directories(tfhttp PRIVATE ${LIBADA_INCLUDE_DIRS})
    target_link_directories(tfhttp PRIVATE ${LIBADA_LIBRARY_DIRS})
    target_link_libraries(tfhttp PRIVATE ${LIBADA_LIBRARIES})
endif()

if(BUILD_STATIC_BINARY OR BUILD_MOSTLY_STATIC_BINARY)
    target_include_directories(tfhttp PRIVATE ${LIBSQLITE3_STATIC_INCLUDE_DIRS})
    target_link_directories(tfhttp PRIVATE ${LIBSQLITE3_STATIC_LIBRARY_DIRS})
    if(BUILD_MOSTLY_STATIC_BINARY)
        target_link_libraries(tfhttp PRIVATE -Wl,-Bstatic;${LIBSQLITE3_STATIC_LIBRARIES};-Wl,-Bdynamic)
    else()
        target_link_libraries(tfhttp PRIVATE ${LIBSQLITE3_STATIC_LIBRARIES})
    endif()
else()
    target_include_directories(tfhttp PRIVATE ${LIBSQLITE3_INCLUDE_DIRS})
    target_link_directories(tfhttp PRIVATE ${LIBSQLITE3_LIBRARY_DIRS})
    target_link_libraries(tfhttp PRIVATE ${LIBSQLITE3_LIBRARIES})
endif()

if(BUILD_STATIC_BINARY OR BUILD_MOSTLY_STATIC_BINARY)
    target_include_directories(tfhttp PRIVATE ${LIBTLS_STATIC_INCLUDE_DIRS})
    target_link_directories(tfhttp PRIVATE ${LIBTLS_STATIC_LIBRARY_DIRS})
    if(BUILD_MOSTLY_STATIC_BINARY)
        target_link_libraries(tfhttp PRIVATE -Wl,-Bstatic;${LIBTLS_STATIC_LIBRARIES};-Wl,-Bdynamic)
    else()
        target_link_libraries(tfhttp PRIVATE ${LIBTLS_STATIC_LIBRARIES})
    endif()
else()
    target_include_directories(tfhttp PRIVATE ${LIBTLS_INCLUDE_DIRS})
    target_link_directories(tfhttp PRIVATE ${LIBTLS_LIBRARY_DIRS})
    target_link_libraries(tfhttp PRIVATE ${LIBTLS_LIBRARIES})
endif()

if(BUILD_MOSTLY_STATIC_BINARY)
    get_target_property(libs tfhttp LINK_LIBRARIES)
    list(REMOVE_ITEM libs m)
    set_target_properties(tfhttp PROPERTIES LINK_LIBRARIES "${libs}")
endif()
