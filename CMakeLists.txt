cmake_minimum_required (VERSION 3.5)
project(SMPTESyncLib)

set_property(GLOBAL PROPERTY POSITION_INDEPENDENT_CODE ON)

# import boost
if(NOT(DEFINED BUILD_SHARED_LIBS))
  set(Boost_USE_STATIC_LIBS ON)
endif()
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS date_time log)
include_directories(${Boost_INCLUDE_DIR})
add_definitions( /DBOOST_ALL_NO_LIB )

# import asdcplib

add_subdirectory(lib/asdcplib)
include_directories(lib/asdcplib/src)

if(WIN32)
	add_definitions(/DKM_WIN32 /D_CONSOLE /DASDCP_PLATFORM=\"win32\" /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
elseif(UNIX)
	add_definitions(/DASDCP_PLATFORM=\"unix\")
endif(WIN32)

# import tinyxml

include_directories(lib/tinyxml)
add_definitions(/DTIXML_USE_STL)
add_library(tinyxml lib/tinyxml/tinystr.cpp lib/tinyxml/tinyxml.cpp lib/tinyxml/tinyxmlerror.cpp lib/tinyxml/tinyxmlparser.cpp)

# library

include_directories(src/AuxData src/client src/commands src/dcisg src/server src/state src/utils src/UUID)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_library(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries ( ${PROJECT_NAME} ${Boost_LIBRARIES} libas02 tinyxml)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

# tests

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

file( GLOB_RECURSE UNIT_TESTS tests/Unit_Tests/*.cpp )

include(GoogleTest)

foreach( UNIT_TEST_PATH ${UNIT_TESTS} )
	get_filename_component(UNIT_TEST_NAME ${UNIT_TEST_PATH} NAME_WE)
  add_executable(${UNIT_TEST_NAME} ${UNIT_TEST_PATH} )
  target_link_libraries( ${UNIT_TEST_NAME} ${PROJECT_NAME} GTest::gtest_main ${Boost_LIBRARIES} libas02 tinyxml)
  gtest_discover_tests( ${UNIT_TEST_NAME})
endforeach( UNIT_TEST_PATH ${UNIT_TESTS} )

