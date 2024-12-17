project( 
    SOP_Pmx
    VERSION 0.0.1
    LANGUAGES CXX
 )

set( library_name SOP_Pmx )

# set the build path
set(SOP_Pmx_BUILD_PATH ${ysyHDKTest_BINARY_DIR}/build/bin/ysyHDKTest)
message(STATUS "SOP_Pmx Build path: ${SOP_Pmx_BUILD_PATH}")

# Add a library and its source files.
add_library( 
    SOP_Pmx SHARED
    src/SOP_Pmx.C
)

# set compile options
target_compile_features(SOP_Pmx PRIVATE cxx_std_17)
set_target_properties(SOP_Pmx PROPERTIES
    # CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY ${SOP_Pmx_BUILD_PATH}/dso
    LIBRARY_OUTPUT_DIRECTORY ${SOP_Pmx_BUILD_PATH}/dso
)

# import HUtils.cmake
include(${ysyHDKTest_SOURCE_DIR}/cmake/HUtils.cmake)

# Convet DsFile to proto.h 
# use build path to ensure out of source
set(SOP_Pmx_PROTOS ${SOP_Pmx_BUILD_PATH}/protos)
generate_proto(
    ${SOP_Pmx_SOURCE_DIR}/src/SOP_Pmx.C
    ${SOP_Pmx_PROTOS}/SOP_Pmx.proto.h
)
target_include_directories(
    SOP_Pmx PRIVATE 
    ${SOP_Pmx_PROTOS}
)

# add target include directories
target_include_directories(SOP_Pmx PRIVATE ${HOUDINI_INC_DIR})

# Link to Houdini
target_link_libraries(SOP_Pmx PRIVATE Houdini)

find_package(glm CONFIG REQUIRED)
target_link_libraries(SOP_Pmx PRIVATE glm::glm)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(SOP_Pmx PRIVATE fmt::fmt-header-only)

