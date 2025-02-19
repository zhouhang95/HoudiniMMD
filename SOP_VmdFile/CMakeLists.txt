project( 
    SOP_VmdFile
    VERSION 0.0.1
    LANGUAGES CXX
 )

set( library_name SOP_VmdFile )

# set the build path
set(SOP_VmdFile_BUILD_PATH ${ysyHDKTest_BINARY_DIR}/build/bin/ysyHDKTest)
message(STATUS "SOP_VmdFile Build path: ${SOP_VmdFile_BUILD_PATH}")

# Add a library and its source files.
add_library( 
    SOP_VmdFile SHARED
    src/SOP_VmdFile.C
)

# set compile options
target_compile_features(SOP_VmdFile PRIVATE cxx_std_17)
set_target_properties(SOP_VmdFile PROPERTIES
    # CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY ${SOP_VmdFile_BUILD_PATH}/dso
    LIBRARY_OUTPUT_DIRECTORY ${SOP_VmdFile_BUILD_PATH}/dso
)

# import HUtils.cmake
include(${ysyHDKTest_SOURCE_DIR}/cmake/HUtils.cmake)

# Convet DsFile to proto.h 
# use build path to ensure out of source
set(SOP_VmdFile_PROTOS ${SOP_VmdFile_BUILD_PATH}/protos)
generate_proto(
    ${SOP_VmdFile_SOURCE_DIR}/src/SOP_VmdFile.C
    ${SOP_VmdFile_PROTOS}/SOP_VmdFile.proto.h
)
target_include_directories(
    SOP_VmdFile PRIVATE 
    ${SOP_VmdFile_PROTOS}
)

# add target include directories
target_include_directories(SOP_VmdFile PRIVATE ${HOUDINI_INC_DIR})

# Link to Houdini
target_link_libraries(SOP_VmdFile PRIVATE Houdini)

find_package(glm CONFIG REQUIRED)
target_link_libraries(SOP_VmdFile PRIVATE glm::glm)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(SOP_VmdFile PRIVATE fmt::fmt-header-only)

