#---------------------------------------------------------------------------
#CMake minimum required version
cmake_minimum_required (VERSION 2.8)
#---------------------------------------------------------------------------
#look up dir's file, and add to var
aux_source_directory(. DIR_SRC)
#---------------------------------------------------------------------------
#generate test
set(EXENAME muinx)
add_executable(${EXENAME} ${DIR_SRC})
#---------------------------------------------------------------------------
#add include path
#include_directories(${LIBRARY_OUTPUT_PATH_INC})
#---------------------------------------------------------------------------
#add lib path
#link_directories(${CMAKE_INSTALL_PREFIX}/src})
#---------------------------------------------------------------------------
#add link lib
set(LIBBASE base)
set(LIBTOOLS tools)
target_link_libraries(${EXENAME} ${LIBBASE} ${LIBTOOLS}  pthread rt)
#---------------------------------------------------------------------------
#install target 
#install(TARGETS json_test DESTINATION ${EXECUTABLE_OUTPUT_PATH})
#---------------------------------------------------------------------------
#add test
#add_test(test_run ${EXECUTABLE_OUTPUT_PATH}/${TESTNAME})
#---------------------------------------------------------------------------
