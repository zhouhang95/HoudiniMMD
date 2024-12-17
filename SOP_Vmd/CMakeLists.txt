project( 
    SOP_Vmd
    VERSION 0.0.1
    LANGUAGES CXX
 )

set( library_name SOP_Vmd )

# set the build path
set(SOP_Vmd_BUILD_PATH ${ysyHDKTest_BINARY_DIR}/build/bin/ysyHDKTest)
message(STATUS "SOP_Vmd Build path: ${SOP_Vmd_BUILD_PATH}")

# Add a library and its source files.
add_library( 
    SOP_Vmd SHARED
    src/SOP_Vmd.C
)

# set compile options
target_compile_features(SOP_Vmd PRIVATE cxx_std_17)
set_target_properties(SOP_Vmd PROPERTIES
    # CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY ${SOP_Vmd_BUILD_PATH}/dso
    LIBRARY_OUTPUT_DIRECTORY ${SOP_Vmd_BUILD_PATH}/dso
)

# import HUtils.cmake
include(${ysyHDKTest_SOURCE_DIR}/cmake/HUtils.cmake)

# Convet DsFile to proto.h 
# use build path to ensure out of source
set(SOP_Vmd_PROTOS ${SOP_Vmd_BUILD_PATH}/protos)
generate_proto(
    ${SOP_Vmd_SOURCE_DIR}/src/SOP_Vmd.C
    ${SOP_Vmd_PROTOS}/SOP_Vmd.proto.h
)
target_include_directories(
    SOP_Vmd PRIVATE 
    ${SOP_Vmd_PROTOS}
)

# add target include directories
target_include_directories(SOP_Vmd PRIVATE ${HOUDINI_INC_DIR})

# Link to Houdini
target_link_libraries(SOP_Vmd PRIVATE Houdini)

find_package(glm CONFIG REQUIRED)
target_link_libraries(SOP_Vmd PRIVATE glm::glm)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(SOP_Vmd PRIVATE fmt::fmt-header-only)

