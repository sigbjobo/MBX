set(BBLOCK_SOURCES system.cpp sys_tools.cpp external_call.cpp subgraphs.cpp)
#set(SUBGRAPHS /home/sigbjobo/MBX/external/subgraphs/subgraphs.cpp)

#add_library(bblock SHARED ${BBLOCK_SOURCES}) 
#target_include_directories(bblock PRIVATE ${CMAKE_SOURCE_DIR}) 
#target_include_directories(bblock PRIVATE ${CMAKE_SOURCE_DIR}/../external/subgraphs/) 

add_library(bblock OBJECT ${BBLOCK_SOURCES})
#add_library(subgraphs OBJECT ${SUBGRAPHS})
target_include_directories(bblock PRIVATE ${CMAKE_SOURCE_DIR}) 
target_include_directories(bblock PRIVATE ${CMAKE_SOURCE_DIR}/../external/)


target_link_libraries(bblock PUBLIC fftw::fftw)
set_property(TARGET bblock PROPERTY POSITION_INDEPENDENT_CODE TRUE)

install(TARGETS bblock
        OBJECTS DESTINATION ${CMAKE_INSTALL_OBJDIR})

#install(TARGETS bblock
#        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/dynamic
#        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
