# - Try to find libada
# Once done this will define
#  ADA_FOUND               - System has libada
#  ADA_INCLUDE_DIRS        - The libada include directories
#  ADA_LIBRARIES           - The libraries needed to use libada

find_path(ADA_INCLUDE_DIR NAMES ada.h)
find_library(ADA_LIBRARY NAMES ada)

if(ADA_INCLUDE_DIR AND EXISTS("${ADA_INCLUDE_DIR}/ada/ada_version.h"))
    file(STRINGS "${ADA_INCLUDE_DIR}/ada/ada_version.h" ADA_VERSION REGEX "^#define[ \t]ADA_VERSION[ \t]\"[^\"]+\"$")
    string(REGEX REPLACE "\"([^\"]+)\"" "\\1" ADA_VERSION "${ADA_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    ada
    REQUIRED_VARS ADA_LIBRARY ADA_INCLUDE_DIR
    VERSION_VAR ADA_VERSION
)

if(ADA_FOUND)
    set(ADA_LIBRARIES ${LIBADA_LIBRARY})
    set(ADA_INCLUDE_DIRS ${LIBADA_INCLUDE_DIR})
endif()

if(ADA_FOUND AND NOT TARGET ada::ada)
    message("Found libada: ${ADA_VERSION} ${ADA_INCLUDE_DIR} ${ADA_LIBRARY}")
    add_library(ada::ada UNKNOWN IMPORTED)
    set_target_properties(ada::ada PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ADA_INCLUDE_DIR}"
        IMPORTED_LOCATION "${ADA_LIBRARY}"
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endif()

mark_as_advanced(ADA_INCLUDE_DIR ADA_LIBRARY)
