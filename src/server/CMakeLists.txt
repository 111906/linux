#定义SRC_LIST变量,包含当前目录下的所有源文件
aux_source_directory(. SRC_LIST) 
aux_source_directory(./db DB_LIST)



# 设置子目录的可执行文件输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
#指定生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST})
#依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base  mysqlclient rdkafka++ pthread)
