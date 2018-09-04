#---------------------------------------------------------------------------
#CMake minimum required version
cmake_minimum_required (VERSION 2.8)
#---------------------------------------------------------------------------
#look up dir's file, and add to var
#aux_source_directory(<dir> <variable>)
aux_source_directory(. DIR_SRCS)
#---------------------------------------------------------------------------
#generate static library
#add_library(libname ${<dir>})
set(LIBNAME net)
add_library(${LIBNAME} ${DIR_SRCS})
#---------------------------------------------------------------------------
#install target 
install(TARGETS ${LIBNAME} DESTINATION ${LIBRARY_OUTPUT_PATH_LIB})

set(HEADERS
    event_loop.h
    )
install(FILES ${HEADERS} DESTINATION ${LIBRARY_OUTPUT_PATH_INC})
#---------------------------------------------------------------------------
