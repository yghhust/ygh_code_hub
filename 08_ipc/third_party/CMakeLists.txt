cmake_minimum_required(VERSION 3.16)

set(MY_LIB "third")

set(LIBS
    "libzmq-static"
    "cppzmq_headers" 
    "nlohmann_json_headers"
)

# zmq
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WITH_PERF_TOOL OFF CACHE BOOL "" FORCE)
set(WITH_DOCS OFF CACHE BOOL "" FORCE)
set(ENABLE_DRAFTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED Off CACHE BOOL "" FORCE)
set(BUILD_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libzmq)

# cppzmq
add_library(cppzmq_headers INTERFACE)
target_include_directories(cppzmq_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cppzmq)

# nlohmann_json
add_library(nlohmann_json_headers INTERFACE)
target_include_directories(nlohmann_json_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann) 

# 整合第三方库
add_library(${MY_LIB} INTERFACE)
target_link_libraries(${MY_LIB} INTERFACE ${LIBS})
target_include_directories(${MY_LIB} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}) 

# ==================== 5. 输出信息 ====================

message(STATUS "========================================")
message(STATUS "Build Configuration:")
message(STATUS "  Binary:  ${MY_LIB}")
message(STATUS "  Libraries: ${LIBS}")
message(STATUS "========================================")
