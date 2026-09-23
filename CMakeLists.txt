cmake_minimum_required(VERSION 3.21)
cmake_policy(VERSION 3.21...3.31)

# #######################################################################################################################
# # Project Defenition
# #######################################################################################################################

project(
	AltTabFix
	VERSION 2.0.0.0
	DESCRIPTION "SKSE plugin that fixes stuck inputs after window loss & focus plus some other fixes"
	LANGUAGES CXX
)

# Not Explosed By Cmake, Manually Set Here for use by version.rc.in
set(PROJECT_COPYRIGHT "GPL-3.0")
set(PROJECT_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(PROJECT_NAME "${PROJECT_NAME}")
set(PROJECT_FRIENDLY_NAME "${PROJECT_NAME}")

# Options
option(PL_STRICT_COMPILE "Enable strict compilation flags (treat warnings as errors)" OFF)
option(PL_DEPLOY_TO_FOLDER "Copy The Built DLL and PDB to the EnvVar Defined Target location" ON)
option(PL_BUILD_DISTRIBUTION "Construct a distribution folder after each build" ON)

# #######################################################################################################################
# # Flags
# #######################################################################################################################

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile_commands.json")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(_ENABLE_EXTENDED_ALIGNED_STORAGE)

# Check if MSVC is the compiler
if(MSVC)
    # Extract the version number of the MSVC compiler
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\1.\\2" MSVC_VERSION_STRING "${MSVC_VERSION}")
    message(STATUS "Detected MSVC version: ${MSVC_VERSION_STRING}")

    # Check if the MSVC version is at least 14.39
    if(MSVC_VERSION LESS 1939)
        message(FATAL_ERROR "MSVC version 14.39 or higher is required. Detected version: ${MSVC_VERSION_STRING}")
    endif()
else()
    message(FATAL_ERROR "This project currently only supports MSVC as the compiler.")
endif()

# #######################################################################################################################
# # Project modules
# #######################################################################################################################
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(xSEPlugin)

# #######################################################################################################################
# # Find dependencies - VCPKG Ports
# #######################################################################################################################

find_path(CLIB_UTIL_INCLUDE_DIRS "ClibUtil/Utils.hpp")

# #######################################################################################################################
# # Find dependencies - VCPKG Other
# #######################################################################################################################

find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)

# #######################################################################################################################
# # Find dependencies - Git
# #######################################################################################################################


# #######################################################################################################################
# # Find dependencies - VCPKG
# #######################################################################################################################

find_package(xbyak CONFIG REQUIRED)
find_package(SimpleIni CONFIG REQUIRED)

# #######################################################################################################################
# # Non-CLib Includes
# #######################################################################################################################

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
	${CLIBUTIL_INCLUDE_DIRS}
	${DETOURS_INCLUDE_DIRS}
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
    ${DETOURS_LIBRARY}
	xbyak::xbyak
	SimpleIni::SimpleIni
)



# #######################################################################################################################
# # Packagging
# #######################################################################################################################
include(Package)

# ----- debug
message(STATUS "CMAKE_CXX_FLAGS:                        ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG:                  ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE:                ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO:         ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS:                 ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS_DEBUG:           ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS_RELEASE:         ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO:  ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")

get_target_property(compile_flags ${PROJECT_NAME} COMPILE_OPTIONS)
message(STATUS "COMPILE_OPTIONS:      ${compile_flags}")

get_target_property(link_flags ${PROJECT_NAME} LINK_OPTIONS)
message(STATUS "LINK_OPTIONS:         ${link_flags}")

