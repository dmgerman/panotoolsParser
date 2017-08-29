# these two variables were not defined for me

cmake_minimum_required(VERSION 3.5)


find_package(FLEX)
find_package(BISON)

#BISON_TARGET(MyParser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
BISON_TARGET(MyParser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
FLEX_TARGET(MyScanner  scanner.l  ${CMAKE_CURRENT_BINARY_DIR}/scanner.c)

ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

##ADD_DEFINITIONS(-std=gnu99)
#ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/parser.c ${CMAKE_CURRENT_BINARY_DIR}/parser.h
#                   COMMAND ${BISON_EXECUTABLE} -d -oparser.c ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
#		   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser.y )

#ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/scanner.c
#                   COMMAND ${FLEX_EXECUTABLE} -oscanner.c ${CMAKE_CURRENT_SOURCE_DIR}/scanner.l
#		   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scanner.l )

# set include path to include the generated parser.h file
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

#build a shared library
ADD_LIBRARY(tparser SHARED tparser.c 
                                  ${BISON_MyParser_OUTPUTS}
                                  ${FLEX_MyScanner_OUTPUTS}
                                 )

#for testing the shared library you probably need some test app too
ADD_EXECUTABLE(testparser testparser.c tparser.c
                                  ${BISON_MyParser_OUTPUTS}
                                  ${FLEX_MyScanner_OUTPUTS}
                                )

#link to the library
TARGET_LINK_LIBRARIES(testparser m)

# installation targets
INSTALL(TARGETS tparser LIBRARY DESTINATION lib)
INSTALL(FILES tparser.h DESTINATION include/tparser)
