# - Try to find libev
# Once done this will define
#  LIBEV_FOUND               - System has libev
#  LIBEV_INCLUDE_DIRS        - The libev include directories
#  LIBEV_LIBRARIES           - The libraries needed to use libev
#  LIBEV_STATIC_INCLUDE_DIRS - The libev static include directories
#  LIBEV_STATIC_LIBRARIES    - The libraries needed to use libev statically

find_path(
    LIBEV_INCLUDE_DIR
    NAMES ev.h
)

set(SAVED_CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES}")
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    find_library(LIBEV_STATIC_LIBRARY NAMES ev)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_SHARED_LIBRARY_SUFFIX}")
    find_library(LIBEV_SHARED_LIBRARY NAMES ev)
set(CMAKE_FIND_LIBRARY_SUFFIXES "${SAVED_CMAKE_FIND_LIBRARY_SUFFIXES}")
unset(SAVED_CMAKE_FIND_LIBRARY_SUFFIXES)

if(LIBEV_SHARED_LIBRARY)
    set(LIBEV_LIBRARY ${LIBEV_SHARED_LIBRARY})
elseif(LIBEV_STATIC_LIBRARY)
    set(LIBEV_LIBRARY ${LIBEV_STATIC_LIBRARY})
endif()

if(LIBEV_INCLUDE_DIR)
    file(
        STRINGS "${LIBEV_INCLUDE_DIR}/ev.h"
        LIBEV_VERSION_MAJOR REGEX "^#define[ \t]+EV_VERSION_MAJOR[ \t]+[0-9]+"
    )

    file(
        STRINGS "${LIBEV_INCLUDE_DIR}/ev.h"
        LIBEV_VERSION_MINOR REGEX "^#define[ \t]+EV_VERSION_MINOR[ \t]+[0-9]+"
    )

    string(REGEX REPLACE "[^0-9]+" "" LIBEV_VERSION_MAJOR "${LIBEV_VERSION_MAJOR}")
    string(REGEX REPLACE "[^0-9]+" "" LIBEV_VERSION_MINOR "${LIBEV_VERSION_MINOR}")
    set(LIBEV_VERSION "${LIBEV_VERSION_MAJOR}.${LIBEV_VERSION_MINOR}")
    unset(LIBEV_VERSION_MINOR)
    unset(LIBEV_VERSION_MAJOR)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    libev
    REQUIRED_VARS LIBEV_LIBRARY LIBEV_INCLUDE_DIR
    VERSION_VAR LIBEV_VERSION
)

if(LIBEV_FOUND)
    if(LIBEV_SHARED_LIBRARY)
        set(LIBEV_LIBRARIES ${LIBEV_SHARED_LIBRARY})
    else()
        set(LIBEV_LIBRARIES ${LIBEV_STATIC_LIBRARY};m)
    endif()

    set(LIBEV_INCLUDE_DIRS  ${LIBEV_INCLUDE_DIR})

    set(LIBEV_STATIC_INCLUDE_DIRS ${LIBEV_INCLUDE_DIRS})
    if(LIBEV_STATIC_LIBRARY)
        set(LIBEV_STATIC_LIBRARIES ${LIBEV_STATIC_LIBRARY};m)
    endif()
endif()

mark_as_advanced(LIBEV_INCLUDE_DIR LIBEV_SHARED_LIBRARY LIBEV_STATIC_LIBRARY)
