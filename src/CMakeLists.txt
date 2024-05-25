if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

option(BUILD_STATIC_BINARY        "Build static binary"         OFF)
option(BUILD_MOSTLY_STATIC_BINARY "All dependencies except system libraries are static" OFF)
option(FORCE_EXTERNAL_LIBEV       "Do not use system libev"     OFF)
option(FORCE_EXTERNAL_LIBADA      "Do not use system libada"    OFF)
option(FORCE_EXTERNAL_LIBLLHTTP   "Do not use system libllhttp" OFF)
option(FORCE_EXTERNAL_SQLITE3     "Do not use system sqlite3"   OFF)
option(FORCE_EXTERNAL_LIBTLS      "Do not use system libtls"    OFF)

include(ExternalProject)
include(FindPkgConfig)
include(FetchContent)

add_executable(tfhttp)
target_compile_options(tfhttp PRIVATE -Wall -Wextra -pedantic)
# target_compile_options(tfhttp PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-pre-c++17-compat -Wno-c++20-compat -Wno-clang-diagnostic-unused-macros)
target_compile_features(tfhttp PRIVATE cxx_std_20)
set_target_properties(
    tfhttp PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

if(BUILD_STATIC_BINARY)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(CMAKE_EXE_LINKER_FLAGS "-static")
    set(BUILD_MOSTLY_STATIC_BINARY OFF)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
elseif(BUILD_MOSTLY_STATIC_BINARY)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
else()
    set(BUILD_SHARED_LIBS ON CACHE INTERNAL "")
endif()
set(orig_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

find_program(CCACHE_EXE ccache)
if(CCACHE_EXE)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_EXE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_EXE})
endif()

target_sources(
    tfhttp
    PRIVATE
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
    find_package(ev)
endif()

if(NOT FORCE_EXTERNAL_LIBADA)
    find_package(ada)
endif()

if(NOT FORCE_EXTERNAL_LIBLLHTTP)
    find_package(llhttp)
endif()

if(NOT FORCE_EXTERNAL_SQLITE3)
    find_package(SQLite3)
endif()

if(NOT FORCE_EXTERNAL_LIBTLS)
    find_package(LibreSSL QUIET)
endif()

find_path(SQLITE3PP sqlite3pp.h)
find_package(nlohmann_json QUIET)

if(NOT EV_FOUND)
    message(STATUS "*** Using external libev")
    set(libev_BUILD_SHARED_LIBS OFF)
    FetchContent_Declare(ev GIT_REPOSITORY https://github.com/sjinks/libev-cmake GIT_TAG v4.33 GIT_SHALLOW ON)
    FetchContent_MakeAvailable(ev)

    target_compile_definitions(ev PRIVATE
        EV_COMPAT3=0
        EV_PREPARE_ENABLE=0
        EV_CHECK_ENABLE=0
        EV_IDLE_ENABLE=0
        EV_FORK_ENABLE=0
        EV_CLEANUP_ENABLE=0
        EV_CHILD_ENABLE=0
        EV_ASYNC_ENABLE=0
        EV_EMBED_ENABLE=0
    )

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(ev PRIVATE EV_VERIFY=3)
    endif()
endif()

if(NOT ADA_FOUND)
    message(STATUS "*** Using external libada")
    set(ADA_BENCHMARKS OFF)
    set(ADA_TOOLS OFF)
    set(ADA_TESTING OFF)
    set(BUILD_SHARED_LIBS OFF)
    FetchContent_Declare(ada GIT_REPOSITORY https://github.com/ada-url/ada.git GIT_TAG v2.7.8 GIT_SHALLOW ON)
    FetchContent_MakeAvailable(ada)
endif()

if(NOT LLHTTP_FOUND)
    message(STATUS "*** Using external libllhttp")
    set(BUILD_SHARED_LIBS OFF)
    set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
    FetchContent_Declare(llhttp URL "https://github.com/nodejs/llhttp/archive/refs/tags/release/v9.2.1.tar.gz")
    FetchContent_MakeAvailable(llhttp)
endif()

if(NOT SQLite3_FOUND)
    message(STATUS "*** Using external sqlite3")
    set(sqlite3_BUILD_SHARED_LIBS OFF)
    FetchContent_Declare(SQLite3 GIT_REPOSITORY https://github.com/sjinks/sqlite3-cmake)
    FetchContent_MakeAvailable(SQLite3)
    target_compile_definitions(SQLite3 PRIVATE
        SQLITE_DQS=0
        SQLITE_DEFAULT_MEMSTATUS=0
        SQLITE_DEFAULT_WAL_SYNCHRONOUS=1
        SQLITE_MAX_EXPR_DEPTH=0
        SQLITE_OMIT_DECLTYPE
        SQLITE_OMIT_DEPRECATED
        SQLITE_OMIT_PROGRESS_CALLBACK
        SQLITE_OMIT_SHARED_CACHE
    )
endif()

if(NOT SQLITE3PP)
    message(STATUS "*** Using external sqlite3pp")
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

if(NOT nlohmann_json_FOUND)
    message(STATUS "*** Using external nlohmann_json")
    FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json GIT_TAG v3.11.3 GIT_SHALLOW ON)
    FetchContent_MakeAvailable(json)
endif()

if(NOT LibreSSL_FOUND)
    message(STATUS "*** Using external libressl")
    set(LIBRESSL_APPS OFF)
    set(LIBRESSL_TESTS OFF)
    set(BUILD_SHARED_LIBS OFF)
    FetchContent_Declare(libressl URL https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.9.2.tar.gz)
    FetchContent_MakeAvailable(libressl)
    add_library(LibreSSL::Crypto ALIAS crypto)
    add_library(LibreSSL::SSL ALIAS ssl)
    add_library(LibreSSL::TLS ALIAS tls)
endif()

set(BUILD_SHARED_LIBS ${orig_BUILD_SHARED_LIBS})
target_link_libraries(tfhttp PRIVATE ev::ev ada::ada llhttp::llhttp nlohmann_json::nlohmann_json SQLite::SQLite3 LibreSSL::TLS)

if(BUILD_MOSTLY_STATIC_BINARY)
    get_target_property(libs tfhttp LINK_LIBRARIES)
    list(REMOVE_ITEM libs m)
    set_target_properties(tfhttp PROPERTIES LINK_LIBRARIES "${libs}")
endif()
