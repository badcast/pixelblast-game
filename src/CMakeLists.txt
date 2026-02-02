cmake_minimum_required(VERSION 3.20)

set(INCL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SOURCE_QRC "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.qrc")
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${INCL_DIR}/*.h")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../.env" CALLBACK_URL)
string(STRIP "${CALLBACK_URL}" CALLBACK_URL)

qt_add_resources(APP_RESOURCES
    ${SOURCE_QRC}
)

add_library(pixelblast SHARED ${SOURCE_FILES} ${APP_RESOURCES})
target_compile_definitions(pixelblast PRIVATE CALLBACK_URL="${CALLBACK_URL}")
target_include_directories(pixelblast PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/ui)
target_include_directories(pixelblast PUBLIC $<BUILD_INTERFACE:${INCL_DIR}>
                                           $<INSTALL_INTERFACE:include>)
target_compile_definitions(pixelblast PRIVATE PB_SHARED)
target_link_libraries(pixelblast PRIVATE Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::Network)
