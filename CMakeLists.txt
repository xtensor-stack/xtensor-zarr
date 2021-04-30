############################################################################
# Copyright (c) Wolf Vollprecht, Johan Mabille, and Sylvain Corlay         #
# Copyright (c) QuantStack                                                 #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.8)
project(xtensor-zarr)

set(XTENSOR_ZARR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(XTENSOR_ZARR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Versionning
# ===========

# Project version
file(STRINGS "${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xtensor_zarr_config.hpp" xtensor_zarr_version_defines
     REGEX "#define XTENSOR_ZARR_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xtensor_zarr_version_defines})
    if(ver MATCHES "#define XTENSOR_ZARR_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XTENSOR_ZARR_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION
    ${XTENSOR_ZARR_VERSION_MAJOR}.${XTENSOR_ZARR_VERSION_MINOR}.${XTENSOR_ZARR_VERSION_PATCH})
message(STATUS "xtensor-zarr version: v${XTENSOR_ZARR_VERSION}")

# Binary version
# See the following URL for explanations about the binary versionning
# https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html#Updating-version-info
file(STRINGS "${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xtensor_zarr_config.hpp" xtensor_zarr_version_defines
    REGEX "#define XTENSOR_ZARR_BINARY_(CURRENT|REVISION|AGE)")
foreach(ver ${xtensor_zarr_version_defines})
    if(ver MATCHES "#define XTENSOR_ZARR_BINARY_(CURRENT|REVISION|AGE) +([^ ]+)$")
        set(XTENSOR_ZARR_BINARY_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(XTENSOR_ZARR_BINARY_VERSION
    ${XTENSOR_ZARR_BINARY_CURRENT}.${XTENSOR_ZARR_BINARY_REVISION}.${XTENSOR_ZARR_BINARY_AGE})
message(STATUS "xtensor-zarr binary version: v${XTENSOR_ZARR_BINARY_VERSION}")

# Build options
# =============

# Compilation options
option(XTENSOR_ZARR_DISABLE_ARCH_NATIVE "disable -march=native flag" OFF)
option(XTENSOR_ZARR_BUILD_SHARED_LIBS "Build xtensor-zarr shared library." ON)
option(XTENSOR_ZARR_BUILD_STATIC_LIBS "Build xtensor-zarr static library (default if BUILD_SHARED_LIBS is OFF)." ON)

# Test options
option(BUILD_TESTS "xtensor-zarr test suite" OFF)
option(DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)
option(DOWNLOAD_GBENCHMARK "download google benchmark and build from source" ON)

# Dependencies
# ============

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")

set(xtensor_REQUIRED_VERSION "0.23.8")
find_package(xtensor ${xtensor_REQUIRED_VERSION} REQUIRED)

set(xtensor_io_REQUIRED_VERSION "0.12.7")
find_package(xtensor-io ${xtensor_io_REQUIRED_VERSION} REQUIRED)

set(zarray_REQUIRED_VERSION "0.1.0")
find_package(zarray ${zarray_REQUIRED_VERSION} REQUIRED)

set(nlohmann_json_REQUIRED_VERSION "3.9.1" )
find_package(nlohmann_json ${nlohmann_json_REQUIRED_VERSION} REQUIRED)

set(GDAL_REQUIRED_VERSION "3.0.0")
find_package(GDAL ${GDAL_REQUIRED_VERSION} REQUIRED)

set(Blosc_REQUIRED_VERSION "1.21.0")
find_package(Blosc ${Blosc_REQUIRED_VERSION} REQUIRED)

set(ZLIB_REQUIRED_VERSION "1.2.11")
find_package(ZLIB ${ZLIB_REQUIRED_VERSION} REQUIRED)

#Remove the following lines when xtensor-io is fixed
include(CMakeFindDependencyMacro)
find_dependency(xtensor REQUIRED)
find_dependency(xtl REQUIRED)

# Build
# =====

set(XTENSOR_ZARR_HEADERS
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xzarr_hierarchy.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xzarr_node.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xzarr_group.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xzarr_array.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xzarr_file_system_store.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xzarr_gcs_store.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xzarr_aws_store.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xzarr_gdal_store.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xzarr_common.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xzarr_compressor.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xzarr_chunked_array.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xtensor_zarr_config.hpp
    ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xtensor_zarr_config_cling.hpp
)

set(XTENSOR_ZARR_GDAL_SOURCE
    ${XTENSOR_ZARR_SOURCE_DIR}/xtensor-zarr-gdal.cpp
)

add_library(xtensor-zarr INTERFACE)
target_link_libraries(xtensor-zarr INTERFACE xtensor-io zarray)

target_include_directories(xtensor-zarr
    INTERFACE
    $<BUILD_INTERFACE:${XTENSOR_ZARR_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(xtensor-zarr
    INTERFACE
    $<BUILD_INTERFACE:${NLOHMANN_JSON_INCLUDE_DIRS}>
)
target_link_libraries(xtensor-zarr
    INTERFACE
    ${NLOHMANN_JSON_LIBRARIES}
)

target_include_directories(xtensor-zarr
    INTERFACE
    $<BUILD_INTERFACE:${CPP_FILESYSTEM_INCLUDE_DIRS}>
)
target_link_libraries(xtensor-zarr
    INTERFACE
    ${CPP_FILESYSTEM_LIBRARIES}
)

target_include_directories(xtensor-zarr
    INTERFACE
    $<BUILD_INTERFACE:${GDAL_INCLUDE_DIRS}>
)
target_link_libraries(xtensor-zarr
    INTERFACE
    ${GDAL_LIBRARIES}
)

target_include_directories(xtensor-zarr
    INTERFACE
    $<BUILD_INTERFACE:${Blosc_INCLUDE_DIRS}>
)
target_link_libraries(xtensor-zarr
    INTERFACE
    ${Blosc_LIBRARIES}
)

target_include_directories(xtensor-zarr
    INTERFACE
    $<BUILD_INTERFACE:${zlib_INCLUDE_DIRS}>
)
target_link_libraries(xtensor-zarr
    INTERFACE
    ${zlib_LIBRARIES}
)

find_package(storage_client)
message(STATUS "Trying to find Google Cloud Storage for GCS IO handler support")
if(${storage_client_FOUND})
    message(STATUS "storage_client ${storage_client_VERSION} found, GCS IO handler support enabled")
    target_include_directories(xtensor-zarr
        INTERFACE
        ${storage_client_INCLUDE_DIRS}
    )
    target_link_libraries(xtensor-zarr
        INTERFACE
        storage_client
    )
else()
    message(WARNING "Google Cloud Storage not found - install google-cloud-cpp for GCS IO handler support")
endif()

if (NOT DEFINED BUILD_SHARED_LIBS)
  set (BUILD_SHARED_LIBS ON)
  find_package(AWSSDK COMPONENTS s3)
  unset (BUILD_SHARED_LIBS)
else ()
  find_package(AWSSDK COMPONENTS s3)
endif ()
message(STATUS "Trying to find AWS SDK for AWS S3 IO handler support")
if(${AWSSDK_FOUND})
  message(STATUS "AWSSDK ${AWSSDK_VERSION} found, AWS S3 IO handler support enabled")
  target_include_directories(xtensor-zarr
      INTERFACE
      ${AWSSDK_INCLUDE_DIRS}
  )
  target_link_libraries(xtensor-zarr
      INTERFACE
      ${AWSSDK_LINK_LIBRARIES}
  )
else()
  message(WARNING "AWSSDK not found - install aws-sdk-cpp for AWS S3 IO handler support")
endif()
message(STATUS "AWSSDK_LINK_LIBRARIES ${AWSSDK_LINK_LIBRARIES}")

if(DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    set(BUILD_TESTS ON)
endif()

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

macro(xtensor_zarr_create_target source target_name linkage output_name)
    string(TOUPPER "${linkage}" linkage_upper)

    if (NOT ${linkage_upper} MATCHES "^(SHARED|STATIC)$")
        message(FATAL_ERROR "Invalid library linkage: ${linkage}")
    endif ()

    # Output
    # ======

    add_library(${target_name} ${linkage_upper} ${source} ${XTENSOR_ZARR_HEADERS})

    if (APPLE)
        set_target_properties(
            ${target_name} PROPERTIES
            MACOSX_RPATH ON
        )
    else ()
        set_target_properties(
            ${target_name} PROPERTIES
            BUILD_WITH_INSTALL_RPATH 1
        )
    endif ()

    target_include_directories(
        ${target_name}
        PUBLIC $<BUILD_INTERFACE:${XTENSOR_ZARR_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(
        ${target_name}
        PUBLIC nlohmann_json::nlohmann_json
        PUBLIC xtl
    )

    set_target_properties(
        ${target_name}
        PROPERTIES
        PUBLIC_HEADER "${XTENSOR_ZARR_HEADERS}"
        COMPILE_DEFINITIONS "XTENSOR_ZARR_EXPORTS"
        PREFIX ""
        VERSION ${XTENSOR_ZARR_BINARY_VERSION}
        SOVERSION ${XTENSOR_ZARR_BINARY_CURRENT}
        OUTPUT_NAME "lib${output_name}"
    )

    # Compilation flags
    # =================

    target_compile_features(${target_name} PRIVATE cxx_std_14)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Intel")

        target_compile_options(${target_name} PUBLIC -Wunused-parameter -Wextra -Wreorder)

        if (NOT XTENSOR_ZARR_DISABLE_ARCH_NATIVE)
            target_compile_options(${target_name} PUBLIC -march=native)
        endif ()

        message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
    endif()

    if (${linkage_upper} STREQUAL "STATIC")
        target_compile_definitions(${target_name} PUBLIC XTENSOR_ZARR_STATIC_LIB)
    endif ()

    if (XTENSOR_ZARR_STATIC_DEPENDENCIES AND CMAKE_DL_LIBS)
        target_link_libraries(${target_name} PRIVATE ${CMAKE_DL_LIBS} util rt)
    endif ()

    target_link_libraries(${target_name}
        PRIVATE
        ${Blosc_LIBRARIES}
    )

    target_link_libraries(${target_name}
        PRIVATE
        ZLIB::ZLIB
    )

endmacro()

set (xtensor_zarr_targets "")
if (XTENSOR_ZARR_BUILD_SHARED_LIBS)
    xtensor_zarr_create_target(${XTENSOR_ZARR_GDAL_SOURCE} xtensor-zarr-gdal SHARED xtensor-zarr-gdal)
    list(APPEND xtensor_zarr_targets xtensor-zarr-gdal)

    target_link_libraries(xtensor-zarr-gdal
        PRIVATE
        ${GDAL_LIBRARIES}
    )
endif ()

# Installation
# ============

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(XTENSOR_ZARR_INSTALL_LIBRARY_DIR "\"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}\"")

configure_file (
    "${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xtensor_zarr_config_cling.hpp.in"
    "${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xtensor_zarr_config_cling.hpp"
)

install(TARGETS ${xtensor_zarr_targets}
    EXPORT ${PROJECT_NAME}-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xtensor-zarr)

install(TARGETS xtensor-zarr
    EXPORT ${PROJECT_NAME}-targets)

# Makes the project importable from the build directory
export(EXPORT ${PROJECT_NAME}-targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

set(XTENSOR_ZARR_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE
    STRING "install path for xtensor-zarrConfig.cmake")

configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XTENSOR_ZARR_CMAKECONFIG_INSTALL_DIR})

# xtensor is header-only and does not depend on the architecture.
# Remove CMAKE_SIZEOF_VOID_P from xtensorConfigVersion.cmake so that an xtensorConfig.cmake
# generated for a 64 bit target can be used for 32 bit targets and vice versa.
set(_XTENSOR_ZARR_CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P})
unset(CMAKE_SIZEOF_VOID_P)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
set(CMAKE_SIZEOF_VOID_P ${_XTENSOR_ZARR_CMAKE_SIZEOF_VOID_P})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${XTENSOR_ZARR_CMAKECONFIG_INSTALL_DIR})

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${XTENSOR_ZARR_CMAKECONFIG_INSTALL_DIR})

configure_file(${PROJECT_NAME}.pc.in
               "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
                @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig/")
