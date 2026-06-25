cmake_minimum_required(VERSION 3.14)
project(cantrip VERSION 1.0.0 LANGUAGES CXX)

add_library(cantrip INTERFACE)

add_library(cantrip::cantrip ALIAS cantrip)

target_include_directories(cantrip INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(cantrip INTERFACE cxx_std_20)

include(GNUInstallDirs)

install(TARGETS cantrip EXPORT cantripTargets)

install(EXPORT cantripTargets
    FILE cantripTargets.cmake
    NAMESPACE cantrip::
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cantrip/cmake
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cantripConfigVersion.cmake"
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cantripConfig.cmake" [[
    include("${CMAKE_CURRENT_LIST_DIR}/cantripTargets.cmake")
]])

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cantripConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cantripConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cantrip/cmake
)
