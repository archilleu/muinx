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
include_directories(../../thirdpart)
#---------------------------------------------------------------------------
#add lib
find_library(LIBBASE base HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdpart/base/lib)
find_library(LIBNET net HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdpart/net/lib)
#---------------------------------------------------------------------------
#add link lib
set(LIBTOOLS tools)
target_link_libraries(${EXENAME} ${LIBNET} ${LIBBASE} ${LIBTOOLS}  pthread rt)
#---------------------------------------------------------------------------
#install target 
install(TARGETS ${EXENAME} DESTINATION ${MUINX_INSTALL_PATH})
#---------------------------------------------------------------------------
