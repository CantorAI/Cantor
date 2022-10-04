set(PROJECT_NAME "Cantor")
project (${PROJECT_NAME})


include_directories(${xlang_sdk_folder})
include_directories("Include")
include_directories("Common")
include_directories("Network")
include_directories("DataView")
include_directories("DataFrame")
include_directories("Core")
include_directories("Log")
include_directories("Main")
include_directories("Database")
include_directories("Tasks")
include_directories("Streaming")

FILE(GLOB XLANG_API_SRC "${xlang_sdk_folder}/*.cpp")
FILE(GLOB CONFIG_FILES ../Config/*.*)
FILE(GLOB SCRIPT_FILES ../Script/*.py)

FILE(GLOB SRC_Common Common/*.cpp)
FILE(GLOB SRC_DataView DataView/*.cpp)
FILE(GLOB SRC_DataFrame DataFrame/frame.cpp 
                DataFrame/tensor.cpp
                DataFrame/FramePack.cpp
                DataFrame/DataFrameWrapper.cpp)
FILE(GLOB SRC_Network Network/*.cpp)
FILE(GLOB SRC_Log Log/*.cpp)
FILE(GLOB SRC_Core Core/*.cpp)
FILE(GLOB SRC_Main Main/*.cpp)
FILE(GLOB SRC_Database Database/*.cpp Database/sqlite/sqlite3.c)
FILE(GLOB SRC_Tasks Tasks/*.cpp)
FILE(GLOB SRC_Streaming Streaming/*.cpp)


add_executable (${PROJECT_NAME}
    ${XLANG_API_SRC}
    ${SRC_Main}
	${SRC_Log}
    ${SRC_DataFrame}
    ${SRC_DataView}
    ${SRC_Core}
    ${SRC_Common}
    ${SRC_Network}
    ${SRC_Database}
    ${SRC_Tasks}
    ${SRC_Streaming}
)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} Ws2_32)
else()
    target_link_libraries(${PROJECT_NAME} pthread dl uuid)
endif()

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory 
	${CMAKE_CURRENT_BINARY_DIR}/Config)
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory 
	${CMAKE_CURRENT_BINARY_DIR}/Script)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
    ${CONFIG_FILES}
    ${CMAKE_CURRENT_BINARY_DIR}/Config/)
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
    ${SCRIPT_FILES}
    ${CMAKE_CURRENT_BINARY_DIR}/Script/)
