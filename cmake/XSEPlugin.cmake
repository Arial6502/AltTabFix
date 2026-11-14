add_compile_definitions(SKYRIM)

include(FetchContent)

FetchContent_Declare(
	CommonLibSSE                   
    GIT_REPOSITORY https://github.com/alandtse/CommonLibSSE-NG/
    GIT_TAG 9955ae3b18dd0aedbebb58fa207a4b7180eb6d67
    GIT_SHALLOW ON
)

# --- Configure options before FetchContent_MakeAvailable
set(REX_OPTION_JSON OFF CACHE BOOL "" FORCE)
set(REX_OPTION_TOML OFF CACHE BOOL "" FORCE)
set(REX_OPTION_INI OFF CACHE BOOL "" FORCE)
set(SKSE_SUPPORT_XBYAK ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_SE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

# --- Make the content available
FetchContent_MakeAvailable(CommonLibSSE)

add_library("${PROJECT_NAME}" SHARED)
set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)

target_compile_features(
	"${PROJECT_NAME}"
	PRIVATE
	cxx_std_23
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(AddCXXFiles)
add_cxx_files("${PROJECT_NAME}")

target_precompile_headers(
	"${PROJECT_NAME}"
	PRIVATE
	src/PCH.hpp
)

# Build DLL RC
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
    @ONLY
)
target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/version.rc")

# Build Version.hpp from project info.
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/src/Version.hpp"
    @ONLY
)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

add_compile_definitions(NOMINMAX)
add_compile_definitions(_UNICODE)

# --- Common compiler options for all configurations ---
target_compile_options(
    "${PROJECT_NAME}"
    PRIVATE
    /MP
    $<$<BOOL:${GTS_STRICT_COMPILE}>:/W4;/WX>
    $<$<NOT:$<BOOL:${GTS_STRICT_COMPILE}>>:/W1>
    /permissive-
    /Zc:alignedNew
    /Zc:auto
    /Zc:__cplusplus
    /Zc:externC
    /Zc:externConstexpr
    /Zc:forScope
    /Zc:hiddenFriend
    /Zc:implicitNoexcept
    /Zc:lambda
    /Zc:noexceptTypes
    /Zc:preprocessor
    /Zc:referenceBinding
    /Zc:rvalueCast
    /Zc:sizedDealloc
    /Zc:strictStrings
    /Zc:ternary
    /Zc:threadSafeInit
    /Zc:trigraphs
    /Zc:wchar_t
    /wd4200 # nonstandard extension used : zero-sized array in struct/union
    /wd4100 # 'identifier' : unreferenced formal parameter
    /wd4101 # 'identifier': unreferenced local variable
    /wd4458 # declaration of 'identifier' hides class member
    /wd4459 # declaration of 'identifier' hides global declaration
    /wd4456 # declaration of 'identifier' hides previous local declaration
    /wd4457 # declaration of 'identifier' hides function parameter
    /wd4189 # 'identifier' : local variable is initialized but not referenced
)

# --- Linker Options ---
target_link_options(
	${PROJECT_NAME}
	PRIVATE
	/WX
)

if(CMAKE_GENERATOR MATCHES "Visual Studio" AND TARGET CommonLibSSE)
    set_target_properties(CommonLibSSE PROPERTIES
        FOLDER "ExternalDependencies"
    )
endif()

find_package(spdlog CONFIG REQUIRED)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/cmake
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC
	CommonLibSSE::CommonLibSSE
)