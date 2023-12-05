cmake_minimum_required(VERSION 3.16)
project(final_autonomous_code_assistant_agent_KZhang2002)

set(CMAKE_CXX_STANDARD 20)

include_directories(Code/lib/curl/include)
link_directories(Code/lib/curl/bin)

add_library(JobSystem SHARED
        Code/JobSystem/JobWorkerThread.cpp
        Code/JobSystem/JobSystem.cpp
        Code/JobSystem/DefaultJobFactory.cpp
        Code/JobSystem/CompileJob.cpp
        Code/JobSystem/RenderJob.cpp
        Code/JobSystem/ChatJob.cpp
        Code/JobSystem/FlowScriptInterpreter.cpp)

target_include_directories(JobSystem PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Code/JobSystem)

target_link_libraries(JobSystem libcurl-x64)

find_package(Threads REQUIRED)

add_executable(final_autonomous_code_assistant_agent_KZhang2002
        Code/main.cpp
        Code/CustomJob.cpp
        Code/CustomJobFactory.cpp)

target_link_libraries(final_autonomous_code_assistant_agent_KZhang2002 PRIVATE JobSystem)
