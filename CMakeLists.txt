cmake_minimum_required(VERSION 3.7)

# Name of the project (will be the name of the plugin)
project(addon)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")
# Build a shared library named after the project from the files in `src/`
file(GLOB SOURCE_FILES "src/*.cc" "src/*.cpp" "src/*.h")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})


# Gives our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Essential include files to build a node addon,
# You should add this line in every CMake.js based project
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
# Include N-API wrappers
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api")

# Essential library files to link to a node addon
# You should add this line in every CMake.js based project
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})


# set(Boost_USE_STATIC_LIBS ON)
# set(Boost_USE_MULTITHREADED ON)  
# set(Boost_USE_STATIC_RUNTIME ON) 
find_package(Boost COMPONENTS date_time filesystem regex atomic chrono log_setup thread log system REQUIRED)
include_directories(${BOOST_INCLUDEDIR})
target_link_libraries(${PROJECT_NAME} 
    Boost::date_time 
    Boost::filesystem 
    Boost::regex 
    Boost::atomic 
    Boost::chrono 
    Boost::log_setup 
    Boost::thread 
    Boost::log 
    Boost::system
    )
    
find_package(OpenCV REQUIRED)
MESSAGE("OpenCV version : ${OpenCV_VERSION}")
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

find_package(dlib REQUIRED)
target_link_libraries(${PROJECT_NAME} dlib::dlib)





     