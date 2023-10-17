cmake_minimum_required(VERSION 3.10)
project(Lab2)

# 使用C++编译器
set(CMAKE_CXX_COMPILER g++)
# 设置编译选项（注意正确的写法）
set(CMAKE_CXX_FLAGS -fexec-charset=GBK)
# 使用C++11标准
add_definitions(-std=c++11)

# 包含头文件路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

# 查找源文件
aux_source_directory(${PROJECT_SOURCE_DIR}/GBN SRC_LIST)

# 设置可执行文件输出路径
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/GBN)

# 创建可执行文件
ADD_EXECUTABLE(GBN ${SRC_LIST})

# 查找并链接 libnetsim.a 库
FIND_LIBRARY(NETSIM_LIB NAMES netsim PATHS ${PROJECT_SOURCE_DIR}/lib)

# 检查是否找到 libnetsim.a 库
if(NETSIM_LIB)
    # 链接 libnetsim.a 库
    TARGET_LINK_LIBRARIES(GBN ${NETSIM_LIB})
else()
    message(FATAL_ERROR "libnetsim.a 库未找到或路径设置不正确")
endif()
