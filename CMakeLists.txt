cmake_minimum_required(VERSION 3.0)
project(testmuduo)

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#配置头文件搜索路径
#include_directories()
#配置库文件搜索路径
#link_directories()

#编译的源文件列表
#set(SOURCE_FILES ./muduo_server.cpp)

#指定搜索的子目录
add_subdirectory(src)

#设置可执行文件的输出目录
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY /home/linu/桌面/聊天器项目/chat/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)


#配置头文件搜索路径
include_directories (${PROJECT_SOURCE_DIR}/include) 
include_directories (${PROJECT_SOURCE_DIR}/include/server) 
include_directories (${PROJECT_SOURCE_DIR}/include/server/db) 
include_directories (${PROJECT_SOURCE_DIR}/thirdparty) 
link_directories(/usr/lib64/mysql)
link_directories(/usr/local/lib)



#把.指定的目录下的所有.cpp文件添加到SOURCE_FILES变量中
#aux_source_directory(. SOURCE_FILES)

#生成可执行文件
#add_executable(muduo_server ${SOURCE_FILES})
#testmudo需要链接的库文件
#target_link_libraries(muduo_server muduo_net muduo_base pthread)
