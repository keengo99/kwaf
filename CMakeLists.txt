cmake_minimum_required(VERSION 2.8.12)
project(kwaf)
file(GLOB KWAF_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(CMAKE_CXX_STANDARD 14)
option(KANGLE_DIR "set kangle dir")

list(APPEND CMAKE_REQUIRED_INCLUDES ${KANGLE_DIR}/include)
list(APPEND CMAKE_REQUIRED_INCLUDES ${KANGLE_DIR}/kasync/include)
list(APPEND CMAKE_REQUIRED_INCLUDES ${KANGLE_DIR}/khttpd/include)
list(APPEND CMAKE_REQUIRED_INCLUDES ${KANGLE_DIR}/khttpd/davlib)

include(CheckIncludeFiles)
check_include_files("ksapi.h;kfeature.h;kforwin32.h" FOUND_KSAPI_HEADER)

if(NOT FOUND_KSAPI_HEADER)
    message(FATAL_ERROR "cann't found ksapi.h. try cmake -DKANGLE_DIR=kangle_dir ${CMAKE_REQUIRED_INCLUDES}")
endif()

list(APPEND KWAF_SOURCE_FILES ${KANGLE_DIR}/khttpd/src/KHttpParser.cpp)
list(APPEND KWAF_SOURCE_FILES ${KANGLE_DIR}/khttpd/src/KStream.cpp)
list(APPEND KWAF_SOURCE_FILES ${KANGLE_DIR}/khttpd/src/KStringBuf.cpp)
list(APPEND KWAF_SOURCE_FILES ${KANGLE_DIR}/src/KSequence.cpp)
list(APPEND KWAF_SOURCE_FILES ${KANGLE_DIR}/src/kmd5.cpp)
list(APPEND KWAF_SOURCE_FILES ${KANGLE_DIR}/kasync/src/krbtree.c)

set_source_files_properties(${KWAF_SOURCE_FILES} PROPERTIES LANGUAGE CXX)


add_library(kwaf MODULE ${KWAF_SOURCE_FILES})
set_property(TARGET kwaf PROPERTY PREFIX "")
target_include_directories(kwaf PUBLIC ${CMAKE_REQUIRED_INCLUDES})
target_link_directories(kwaf PUBLIC ${KWEB_LIBS_DIR})
set_target_properties(kwaf PROPERTIES LINKER_LANGUAGE CXX)

set(CMAKE_INSTALL_PREFIX ${KANGLE_DIR})
install(TARGETS kwaf LIBRARY DESTINATION bin)
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/
    DESTINATION "ext"
)
include(CPack)
set(CPACK_PACKAGE_NAME "kwaf")