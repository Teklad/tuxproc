get_filename_component(TuxProc_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(CMakeFindDependencyMacro)

list(APPEND CMAKE_MODULE_PATH ${TuxProc_CMAKE_DIR})
list(REMOVE_AT CMAKE_MODULE_PATH -1)

if(NOT TARGET TuxProc::TuxProc)
    include("${TuxProc_CMAKE_DIR}/TuxProcTargets.cmake")
endif()

set(TuxProc_LIBRARIES TuxProc::TuxProc)
