project( 
    SOP_MMDBoneDeform
    VERSION 0.0.1
    LANGUAGES CXX
 )

set( library_name SOP_MMDBoneDeform )

# set the build path
set(SOP_MMDBoneDeform_BUILD_PATH ${ysyHDKTest_BINARY_DIR}/build/bin/ysyHDKTest)
message(STATUS "SOP_MMDBoneDeform Build path: ${SOP_MMDBoneDeform_BUILD_PATH}")

# Add a library and its source files.
add_library( 
    SOP_MMDBoneDeform SHARED
    src/SOP_MMDBoneDeform.C
)

# set compile options
target_compile_features(SOP_MMDBoneDeform PRIVATE cxx_std_17)
set_target_properties(SOP_MMDBoneDeform PROPERTIES
    # CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY ${SOP_MMDBoneDeform_BUILD_PATH}/dso
    LIBRARY_OUTPUT_DIRECTORY ${SOP_MMDBoneDeform_BUILD_PATH}/dso
)

# import HUtils.cmake
include(${ysyHDKTest_SOURCE_DIR}/cmake/HUtils.cmake)

# Convet DsFile to proto.h 
# use build path to ensure out of source
set(SOP_MMDBoneDeform_PROTOS ${SOP_MMDBoneDeform_BUILD_PATH}/protos)
generate_proto(
    ${SOP_MMDBoneDeform_SOURCE_DIR}/src/SOP_MMDBoneDeform.C
    ${SOP_MMDBoneDeform_PROTOS}/SOP_MMDBoneDeform.proto.h
)
target_include_directories(
    SOP_MMDBoneDeform PRIVATE 
    ${SOP_MMDBoneDeform_PROTOS}
)

# add target include directories
target_include_directories(SOP_MMDBoneDeform PRIVATE ${HOUDINI_INC_DIR})

# Link to Houdini
target_link_libraries(SOP_MMDBoneDeform PRIVATE Houdini)

find_package(glm CONFIG REQUIRED)
target_link_libraries(SOP_MMDBoneDeform PRIVATE glm::glm)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(SOP_MMDBoneDeform PRIVATE fmt::fmt-header-only)

