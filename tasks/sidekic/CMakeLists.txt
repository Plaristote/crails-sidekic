cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE task_files
     *.cpp
     *.cxx)

add_executable(sidekic ${task_files})

target_link_libraries(sidekic ${dependencies})

set_target_properties(sidekic PROPERTIES OUTPUT_NAME "task")

