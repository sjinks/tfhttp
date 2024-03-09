# - Try to find llhttp
# Once done this will define
#  LLHTTP_FOUND               - System has llhttp
#  LLHTTP_INCLUDE_DIRS        - The llhttp include directories
#  LLHTTP_LIBRARIES           - The libraries needed to use llhttp

pkg_check_modules(LLHTTP QUIET IMPORTED_TARGET libllhttp)
if (NOT LLHTTP_FOUND)
    find_path(LLHTTP_INCLUDE_DIR NAMES llhttp.h)

    find_library(LLHTTP_LIBRARY NAMES llhttp)
    if(LLHTTP_INCLUDE_DIR AND EXISTS("${LLHTTP_INCLUDE_DIR}/llhttp.h"))
        file(STRINGS "${LLHTTP_INCLUDE_DIR}/llhttp.h" LLHTTP_VERSION_MAJOR REGEX "^#define[ \t]+LLHTTP_VERSION_MAJOR[ \t]+[0-9]+")
        file(STRINGS "${LLHTTP_INCLUDE_DIR}/llhttp.h" LLHTTP_VERSION_MINOR REGEX "^#define[ \t]+LLHTTP_VERSION_MINOR[ \t]+[0-9]+")
        string(REGEX REPLACE "[^0-9]+" "" LLHTTP_VERSION_MAJOR "${LLHTTP_VERSION_MAJOR}")
        string(REGEX REPLACE "[^0-9]+" "" LLHTTP_VERSION_MINOR "${LLHTTP_VERSION_MINOR}")
        set(LLHTTP_VERSION "${LLHTTP_VERSION_MAJOR}.${LLHTTP_VERSION_MINOR}")
        unset(LLHTTP_VERSION_MINOR)
        unset(LLHTTP_VERSION_MAJOR)
    endif()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(
        llhttp
        REQUIRED_VARS LLHTTP_LIBRARY LLHTTP_INCLUDE_DIR
        VERSION_VAR LLHTTP_VERSION
    )

    if(LLHTTP_FOUND)
        set(LLHTTP_LIBRARIES ${LLHTTP_LIBRARY})
        set(LLHTTP_INCLUDE_DIRS  ${LLHTTP_INCLUDE_DIR})
    endif()

    mark_as_advanced(LLHTTP_INCLUDE_DIR LLHTTP_LIBRARY)

    if(LLHTTP_FOUND AND NOT TARGET llhttp::llhttp)
        message("Found llhttp: ${LLHTTP_VERSION} (found version ${LLHTTP_VERSION}) ${LLHTTP_INCLUDE_DIR} ${LLHTTP_LIBRARY}")
        add_library(llhttp::llhttp UNKNOWN IMPORTED)
        set_target_properties(llhttp::llhttp PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${LLHTTP_INCLUDE_DIR}"
            IMPORTED_LOCATION "${LLHTTP_LIBRARY}"
        )
    endif()
else()
    add_library(llhttp::llhttp ALIAS PkgConfig::LLHTTP)
endif()
