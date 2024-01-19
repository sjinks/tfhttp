# - Try to find libev
# Once done this will define
#  LIBSQLITE3_FOUND               - System has libsqlite3
#  LIBSQLITE3_INCLUDE_DIRS        - The libsqlite3 include directories
#  LIBSQLITE3_LIBRARIES           - The libraries needed to use libsqlite3
#  LIBSQLITE3_STATIC_INCLUDE_DIRS - The libsqlite3 static include directories
#  LIBSQLITE3_STATIC_LIBRARIES    - The libraries needed to use libsqlite3 statically

pkg_check_modules(LIBSQLITE3 QUIET sqlite3)
if (NOT LIBSQLITE3_FOUND)
    find_path(
        LIBSQLITE3_INCLUDE_DIR
        NAMES sqlite3.h
    )

    set(SAVED_CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES}")
        set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
        find_library(LIBSQLITE3_STATIC_LIBRARY NAMES sqlite3)
        set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_SHARED_LIBRARY_SUFFIX}")
        find_library(LIBSQLITE3_SHARED_LIBRARY NAMES sqlite3)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${SAVED_CMAKE_FIND_LIBRARY_SUFFIXES}")

    if(LIBSQLITE3_SHARED_LIBRARY)
        set(LIBSQLITE3_LIBRARY ${LIBSQLITE3_SHARED_LIBRARY})
    elseif(LIBSQLITE3_STATIC_LIBRARY)
        set(LIBSQLITE3_LIBRARY ${LIBSQLITE3_STATIC_LIBRARY})
    endif()

    if(LIBSQLITE3_INCLUDE_DIR)
        file(STRINGS ${LIBSQLITE3_INCLUDE_DIR}/sqlite3.h _ver_line REGEX "^#define SQLITE_VERSION  *\"[0-9]+\\.[0-9]+\\.[0-9]+\"" LIMIT_COUNT 1)
        string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" LIBSQLITE3_VERSION "${_ver_line}")
        unset(_ver_line)
    endif()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(
        libsqlite3
        REQUIRED_VARS LIBSQLITE3_LIBRARY LIBSQLITE3_INCLUDE_DIR
        VERSION_VAR LIBSQLITE3_VERSION
    )

    if(LIBSQLITE3_FOUND)
        if(LIBSQLITE3_SHARED_LIBRARY)
            set(LIBSQLITE3_LIBRARIES ${LIBSQLITE3_SHARED_LIBRARY})
        else()
            set(LIBSQLITE3_LIBRARIES ${LIBSQLITE3_STATIC_LIBRARY})
        endif()

        set(LIBSQLITE3_INCLUDE_DIRS  ${LIBSQLITE3_INCLUDE_DIR})

        set(LIBSQLITE3_STATIC_INCLUDE_DIRS ${LIBSQLITE3_INCLUDE_DIRS})
        if(LIBSQLITE3_STATIC_LIBRARY)
            set(LIBSQLITE3_STATIC_LIBRARIES ${LIBSQLITE3_STATIC_LIBRARY})
        endif()
    endif()

    mark_as_advanced(LIBSQLITE3_INCLUDE_DIR LIBSQLITE3_SHARED_LIBRARY LIBSQLITE3_STATIC_LIBRARY)
endif()
