# - Try to find libev
# Once done this will define
#  EV_FOUND               - System has libev
#  EV_INCLUDE_DIRS        - The libev include directories
#  EV_LIBRARIES           - The libraries needed to use libev

find_path(EV_INCLUDE_DIR NAMES ev.h)
find_library(EV_LIBRARY NAMES ev)
find_library(LIBM_LIBRARY NAMES m)

if(EV_INCLUDE_DIR AND EXISTS("${EV_INCLUDE_DIR}/ev.h"))
    file(
        STRINGS "${EV_INCLUDE_DIR}/ev.h"
        EV_VERSION_MAJOR REGEX "^#define[ \t]+EV_VERSION_MAJOR[ \t]+[0-9]+"
    )

    file(
        STRINGS "${EV_INCLUDE_DIR}/ev.h"
        EV_VERSION_MINOR REGEX "^#define[ \t]+EV_VERSION_MINOR[ \t]+[0-9]+"
    )

    string(REGEX REPLACE "[^0-9]+" "" EV_VERSION_MAJOR "${EV_VERSION_MAJOR}")
    string(REGEX REPLACE "[^0-9]+" "" EV_VERSION_MINOR "${EV_VERSION_MINOR}")
    set(EV_VERSION "${EV_VERSION_MAJOR}.${EV_VERSION_MINOR}")
    unset(EV_VERSION_MINOR)
    unset(EV_VERSION_MAJOR)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    ev
    REQUIRED_VARS EV_LIBRARY EV_INCLUDE_DIR
    VERSION_VAR EV_VERSION
)

if(EV_FOUND)
    set(EV_LIBRARIES ${EV_LIBRARY})
    if(LIBM_LIBRARY)
        list(APPEND EV_LIBRARIES ${LIBM_LIBRARY})
    endif()
    set(EV_INCLUDE_DIRS ${EV_INCLUDE_DIR})
endif()

if(EV_FOUND AND NOT TARGET ev::ev)
    add_library(ev::ev UNKNOWN IMPORTED)
    set_target_properties(ev::ev PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${EV_INCLUDE_DIR}"
        IMPORTED_LOCATION "${EV_LIBRARY}"
        INTERFACE_LINK_LIBRARIES "$<$<BOOL:${LIBM_LIBRARY}>:${LIBM_LIBRARY}>"
    )
endif()

mark_as_advanced(EV_INCLUDE_DIR EV_LIBRARY)
