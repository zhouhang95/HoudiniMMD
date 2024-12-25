cmake_minimum_required(VERSION 3.6)
project(
    ysyHDKTest 
    VERSION 0.0.1 
    DESCRIPTION "some HDK test"
)

# 设置 msvc 编译选项
if (MSVC)
    # 使用 /utf-8 编译选项
    add_compile_options("/utf-8")
endif()

# set variables for HDK
set(HYTHON $ENV{HFS}/bin/hython)
set(HDK_CMAKE $ENV{HFS}/toolkit/cmake)
set(HOUDINI_INC_DIR $ENV{HFS}/toolkit/include)
if (MSVC)
    set(HOUDINI_LIBRARY_DIR $ENV{HFS}/custom/houdini/dsolib)
elseif(UNIX)
    set(HOUDINI_LIBRARY_DIR $ENV{HFS}/dsolib)
endif()
message(STATUS "HFS: $ENV{HFS}")
message(STATUS "HYTHON: ${HYTHON}")
message(STATUS "HDK cmake: ${HDK_CMAKE}")
message(STATUS "dsolib path: ${HOUDINI_LIBRARY_DIR}")
list(APPEND CMAKE_PREFIX_PATH "${HDK_CMAKE}")
find_package(Houdini REQUIRED)

# 添加子目录
add_subdirectory(SOP_Pmx)
add_subdirectory(SOP_Vmd)
add_subdirectory(SOP_MMDBoneDeform)