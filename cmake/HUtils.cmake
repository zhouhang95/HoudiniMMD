# find all lib of houdini
function(H_find_all_libraries DIR VAR)
    message(STATUS "Finding libraries in ${DIR}")
    file(GLOB_RECURSE LIB_FILES
        "${DIR}/*.lib"
        "${DIR}/*.a"
        "${DIR}/*.dylib"
        "${DIR}/*.so"
        "${DIR}/*.dll"
    )
    set(${VAR} ${LIB_FILES} PARENT_SCOPE)
endfunction()

# set pointer size for marco
function(H_set_pointer_size)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_compile_definitions(SIZEOF_VOID_P=8)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        add_compile_definitions(SIZEOF_VOID_P=4)
    else()
        message(FATAL_ERROR "Unknown pointer size")
    endif()
endfunction()

# generate_proto
# if INPUT_FILE changed, need to run cmake config again
# this will invoke hython to do it
# we can just use Houini.cmake to do it as well
# it's up to you
function(generate_proto INPUT_FILE OUTPUT_FILE)
    if(NOT DEFINED HYTHON OR HYTHON STREQUAL "")
        message(FATAL_ERROR "HYTHON is not set or is empty. Please define HYTHON.")
    endif()

    # 确保输出文件目录已创建
    get_filename_component(OUTPUT_DIR ${OUTPUT_FILE} DIRECTORY)
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    # 使用 execute_process 在配置阶段执行命令
    execute_process(
        COMMAND ${HYTHON} -m generate_proto ${INPUT_FILE} ${OUTPUT_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}  # 可选：设置工作目录
        RESULT_VARIABLE HYTHON_RESULT                  # 存储执行结果（0 表示成功）
        OUTPUT_VARIABLE HYTHON_OUTPUT                  # 存储命令的输出
        ERROR_VARIABLE HYTHON_ERROR                    # 存储命令的错误信息
    )

    # 检查执行结果
    if(NOT HYTHON_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to run hython: ${HYTHON_ERROR}")
    else()
        message(STATUS "Hython ran successfully: ${HYTHON_OUTPUT}")
    endif()
endfunction()