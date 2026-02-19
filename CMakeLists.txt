cmake_minimum_required(VERSION 3.19)
project(pixelblast LANGUAGES CXX)

find_package(Qt6 6 REQUIRED COMPONENTS Core Widgets Multimedia Network)

qt_standard_project_setup()

add_subdirectory(src)

qt_add_resources(APP_RESOURCES
    MainResource.qrc
)

qt_add_executable(pixelblast_client
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    ${APP_RESOURCES}
)

target_link_libraries(pixelblast_client
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        pixelblast
)

include(GNUInstallDirs)

install(TARGETS pixelblast_client
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET pixelblast_client
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/Names.txt" NICKNAMES_RAW)

target_compile_definitions(pixelblast_client PRIVATE NICKNAMES_RAW="${NICKNAMES_RAW}")

