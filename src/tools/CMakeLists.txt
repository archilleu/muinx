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
set(LIBTOOLS tools)
add_library(${LIBTOOLS} ${DIR_SRCS})
#---------------------------------------------------------------------------
#install target 
install(TARGETS ${LIBTOOLS} DESTINATION ${LIBRARY_OUTPUT_PATH_LIB})

set(HEADERS
    muinx_logger.h
    )
install(FILES ${HEADERS} DESTINATION ${LIBRARY_OUTPUT_PATH_INC})
#---------------------------------------------------------------------------
