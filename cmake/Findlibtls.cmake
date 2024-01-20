# - Try to find libev
# Once done this will define
#  LIBTLS_FOUND               - System has libtls
#  LIBTLS_INCLUDE_DIRS        - The libtls include directories
#  LIBTLS_LIBRARIES           - The libraries needed to use libtls
#  LIBTLS_STATIC_INCLUDE_DIRS - The libtls static include directories
#  LIBTLS_STATIC_LIBRARIES    - The libraries needed to use libtls statically

pkg_check_modules(LIBTLS QUIET libtls)
if (NOT LIBTLS_FOUND)
    find_path(
        LIBTLS_INCLUDE_DIR
        NAMES libtls.h
    )

    set(SAVED_CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES}")
        set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
        find_library(LIBTLS_STATIC_LIBRARY NAMES libtls)
        set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_SHARED_LIBRARY_SUFFIX}")
        find_library(LIBTLS_SHARED_LIBRARY NAMES libtls)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${SAVED_CMAKE_FIND_LIBRARY_SUFFIXES}")
    unset(SAVED_CMAKE_FIND_LIBRARY_SUFFIXES)

    if(LIBTLS_SHARED_LIBRARY)
        set(LIBTLS_LIBRARY ${LIBTLS_SHARED_LIBRARY})
    elseif(LIBTLS_STATIC_LIBRARY)
        set(LIBTLS_LIBRARY ${LIBTLS_STATIC_LIBRARY})
    endif()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(
        liblibtls
        REQUIRED_VARS LIBTLS_LIBRARY LIBTLS_INCLUDE_DIR
    )

    if(LIBTLS_FOUND)
        if(LIBTLS_SHARED_LIBRARY)
            set(LIBTLS_LIBRARIES ${LIBTLS_SHARED_LIBRARY})
        else()
            set(LIBTLS_LIBRARIES ${LIBTLS_STATIC_LIBRARY})
        endif()

        set(LIBTLS_INCLUDE_DIRS  ${LIBTLS_INCLUDE_DIR})

        set(LIBTLS_STATIC_INCLUDE_DIRS ${LIBTLS_INCLUDE_DIRS})
        if(LIBTLS_STATIC_LIBRARY)
            set(LIBTLS_STATIC_LIBRARIES ${LIBTLS_STATIC_LIBRARY})
        endif()
    endif()

    mark_as_advanced(LIBTLS_INCLUDE_DIR LIBTLS_SHARED_LIBRARY LIBTLS_STATIC_LIBRARY)
endif()
