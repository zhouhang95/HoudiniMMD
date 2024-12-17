#!/bin/bash
echo "Defining HFS environment variables..."
# Set HFS environment variables
# export HFS="/opt/hfs20.0.582"

# 运行 CMake 配置命令
echo "Running CMake with Ninja generator..."
cmake -G Ninja -B target -DCMAKE_EXPORT_COMPILE_COMMANDS=ON 