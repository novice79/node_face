cmake_minimum_required(VERSION 3.2)
set(CMAKE_TOOLCHAIN_FILE "D:/installed/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Name of the project (will be the name of the plugin)
project(addon)
message ("aaaaaaaaaaaaaaaaaaaaaaaaa ${VCPKG_TARGET_TRIPLET}")
if (MSVC)
    # For precompiled header.
    # Set 
    # "Precompiled Header" to "Use (/Yu)"
    # "Precompiled Header File" to "stdafx.h"
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Yustdafx.h /FIstdafx.h")
endif()
set_source_files_properties(src/stdafx.cpp
    PROPERTIES
    COMPILE_FLAGS "/Ycstdafx.h"
)
# set(BOOST_ROOT "E:/cpp_libs/boost/win64")
# set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include/boost-1_66")
# set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
# FOREACH(LIB 
#     libboost_system-vc140-mt-s-x64-1_66.lib
#     libboost_date_time-vc140-mt-s-x64-1_66.lib
#     libboost_regex-vc140-mt-s-x64-1_66.lib
#     libboost_atomic-vc140-mt-s-x64-1_66.lib
#     libboost_chrono-vc140-mt-s-x64-1_66.lib
#     libboost_log_setup-vc140-mt-s-x64-1_66.lib
#     libboost_thread-vc140-mt-s-x64-1_66.lib
#     libboost_filesystem-vc140-mt-s-x64-1_66.lib
#     libboost_log-vc140-mt-s-x64-1_66.lib
# )
#     FIND_LIBRARY(FOUND_LIB_${LIB} ${LIB} "${BOOST_LIBRARYDIR}")
#     LIST(APPEND Boost_LIBRARIES ${FOUND_LIB_${LIB}})
# ENDFOREACH(LIB)
find_package(Boost COMPONENTS date_time filesystem regex atomic chrono log_setup thread log system REQUIRED)
# opencv begin ####################################
SET(OpenCV_DIR "E:/cpp_libs/opencv/build/x64/vc14/lib" )
# Requires OpenCV
FIND_PACKAGE( OpenCV 3.0.0 REQUIRED )
MESSAGE("OpenCV version : ${OpenCV_VERSION}")
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})
# opencv end ####################################

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

# Build a shared library named after the project from the files in `src/`
file(GLOB SOURCE_FILES "src/*.cc" "src/*.cpp" "src/*.h")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})


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

# MESSAGE("@@@@@@@ boost libs : ${Boost_LIBRARIES} @@@@@@@")
include_directories(${BOOST_INCLUDEDIR})
#link_directories(${BOOST_LIBRARYDIR})

# target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
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
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )