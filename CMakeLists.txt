cmake_minimum_required(VERSION 3.19)
project(pixelblast LANGUAGES CXX)

find_package(Qt6 6 REQUIRED COMPONENTS Core Widgets Multimedia)

qt_standard_project_setup()

add_subdirectory(src)

qt_add_executable(pixelblastexec
    WIN32 MACOSX_BUNDLE
    main.cpp
)

target_link_libraries(pixelblastexec
    PRIVATE
        Qt::Core
        Qt::Widgets pixelblast
)

include(GNUInstallDirs)

install(TARGETS pixelblastexec
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET pixelblastexec
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
