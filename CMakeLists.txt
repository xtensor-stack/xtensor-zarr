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

# Configuration
# =============

configure_file (
    "${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xtensor_zarr_config.hpp.in"
    "${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xtensor_zarr_config.hpp"
)

# Versionning
# ===========

file(STRINGS "${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xtensor_zarr_config.hpp" xtensor_zarr_version_defines
     REGEX "#define XTENSOR_ZARR_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xtensor_zarr_version_defines})
    if(ver MATCHES "#define XTENSOR_ZARR_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XTENSOR_ZARR_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION
    ${XTENSOR_ZARR_VERSION_MAJOR}.${XTENSOR_ZARR_VERSION_MINOR}.${XTENSOR_ZARR_VERSION_PATCH})
message(STATUS "Building xtensor-zarr v${${PROJECT_NAME}_VERSION}")

# Dependencies
# ============

find_package(xtensor 0.21.2 REQUIRED)
find_package(xtl 0.6.9 REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

message(STATUS "Found xtensor: ${xtensor_INCLUDE_DIRS}/xtensor")

# Build
# =====

set(XTENSOR_ZARR_HEADERS
  ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xtensor-zarr.hpp
  ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/hierarchy.hpp
  ${XTENSOR_ZARR_INCLUDE_DIR}/xtensor-zarr/xtensor_zarr_config.hpp
)

OPTION(BUILD_TESTS "xtensor-zarr test suite" OFF)
OPTION(DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)
OPTION(DOWNLOAD_GBENCHMARK "download google benchmark and build from source" ON)

add_library(xtensor-zarr INTERFACE)

include_directories(${GTEST_INCLUDE_DIRS})

target_include_directories(xtensor-zarr
  INTERFACE
    $<BUILD_INTERFACE:${XTENSOR_ZARR_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(xtensor-zarr
  INTERFACE
    xtl
    xtensor
    nlohmann_json::nlohmann_json
)

if(DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    set(BUILD_TESTS ON)
endif()

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

# Installation
# ============

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS xtensor-zarr
        EXPORT ${PROJECT_NAME}-targets)

export(EXPORT ${PROJECT_NAME}-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

install(FILES ${XTENSOR_ZARR_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xtensor-zarr)

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
