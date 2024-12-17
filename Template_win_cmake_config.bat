echo off
echo define HFS environment variables...
@REM Set HFS environment variables
@REM set HFS=G:/Program Files/Side Effects Software/Houdini 20.5.370

REM 运行CMake配置命令
echo Running CMake with Ninja generator...
cmake -G Ninja -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON