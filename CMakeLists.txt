cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
project(tuxproc VERSION 1.0.0 LANGUAGES CXX)


# Make sure that custom modules are found
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Add needed includes
include(BuildType)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Create target
add_library(tuxproc SHARED
    src/process.cpp
    src/region.cpp
)

# Add an alias so that the library can be used inside the build tree
add_library(TuxProc::tuxproc ALIAS tuxproc)

# Configure target properties
target_include_directories(tuxproc
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(tuxproc PRIVATE cxx_std_11)
target_compile_options(tuxproc PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>)
set_target_properties(tuxproc PROPERTIES EXPORT_NAME TuxProc)

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/TuxProc)
set(INSTALL_PKGCONFIGDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")

install(TARGETS tuxproc
    EXPORT tuxproc-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT tuxproc-targets
    FILE TuxProcTargets.cmake
    NAMESPACE TuxProc::
    DESTINATION ${INSTALL_CONFIGDIR}
)

# pkgconfig and cmake files distribution
# pkgconfig
set(TUXPROC_PC "${CMAKE_CURRENT_BINARY_DIR}/tuxproc.pc")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/tuxproc.pc.cmakein"
    "${TUXPROC_PC}" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tuxproc.pc"
    DESTINATION "${INSTALL_PKGCONFIGDIR}")

# cmake
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/TuxProcConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/cmake/TuxProcConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/TuxProcConfig.cmake"
    INSTALL_DESTINATION "${INSTALL_CONFIGDIR}"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/TuxProcConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/TuxProcConfigVersion.cmake"
    DESTINATION "${INSTALL_CONFIGDIR}"
)

# Export from the build tree
export(EXPORT tuxproc-targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/TuxProcTargets.cmake"
    NAMESPACE TuxProc::
)

export(PACKAGE TuxProc)
