# - Try to find libev
# Once done this will define
#  LIBADA_FOUND               - System has libada
#  LIBADA_INCLUDE_DIRS        - The libada include directories
#  LIBADA_LIBRARIES           - The libraries needed to use libada
#  LIBADA_STATIC_INCLUDE_DIRS - The libada static include directories
#  LIBADA_STATIC_LIBRARIES    - The libraries needed to use libada statically

find_path(
    LIBADA_INCLUDE_DIR
    NAMES ada.h
)

set(SAVED_CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES}")
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    find_library(LIBADA_STATIC_LIBRARY NAMES ada)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_SHARED_LIBRARY_SUFFIX}")
    find_library(LIBADA_SHARED_LIBRARY NAMES ada)
set(CMAKE_FIND_LIBRARY_SUFFIXES "${SAVED_CMAKE_FIND_LIBRARY_SUFFIXES}")
unset(SAVED_CMAKE_FIND_LIBRARY_SUFFIXES)

if(LIBADA_SHARED_LIBRARY)
    set(LIBADA_LIBRARY ${LIBADA_SHARED_LIBRARY})
elseif(LIBADA_STATIC_LIBRARY)
    set(LIBADA_LIBRARY ${LIBADA_STATIC_LIBRARY})
endif()

if(LIBADA_INCLUDE_DIR)
    file(STRINGS "${LIBADA_INCLUDE_DIR}/ada/ada_version.h" LIBADA_VERSION REGEX "^#define[ \t]ADA_VERSION[ \t]\"[^\"]+\"$")
    string(REGEX REPLACE "\"([^\"]+)\"" "\\1" LIBADA_VERSION "${LIBADA_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    libada
    REQUIRED_VARS LIBADA_LIBRARY LIBADA_INCLUDE_DIR
    VERSION_VAR LIBADA_VERSION
)

if(LIBADA_FOUND)
    if(LIBADA_SHARED_LIBRARY)
        set(LIBADA_LIBRARIES ${LIBADA_SHARED_LIBRARY})
    else()
        set(LIBADA_LIBRARIES ${LIBADA_STATIC_LIBRARY})
    endif()

    set(LIBADA_INCLUDE_DIRS  ${LIBADA_INCLUDE_DIR})

    set(LIBADA_STATIC_INCLUDE_DIRS ${LIBADA_INCLUDE_DIRS})
    if(LIBADA_STATIC_LIBRARY)
        set(LIBADA_STATIC_LIBRARIES ${LIBADA_STATIC_LIBRARY})
    endif()
endif()

mark_as_advanced(LIBADA_INCLUDE_DIR LIBADA_SHARED_LIBRARY LIBADA_STATIC_LIBRARY)
