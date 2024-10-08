cmake_minimum_required(VERSION 3.16)

project(VcmxTo3D VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add required Qt components, including 3D modules
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets 3DCore 3DRender 3DExtras 3DInput)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets 3DCore 3DRender 3DExtras 3DInput)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    src.qrc  # Add your resource file
)

# For Qt 6 and above
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(VcmxTo3D
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    # For Qt 5 or earlier
    if(ANDROID)
        add_library(VcmxTo3D SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(VcmxTo3D
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Link libraries including Qt's 3D modules
target_link_libraries(VcmxTo3D PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets
                                Qt${QT_VERSION_MAJOR}::3DCore Qt${QT_VERSION_MAJOR}::3DRender Qt${QT_VERSION_MAJOR}::3DExtras
                                Qt${QT_VERSION_MAJOR}::3DInput)

# MacOS, Windows, iOS properties
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.VcmxTo3D)
endif()
set_target_properties(VcmxTo3D PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# GNU Install directories for installation
include(GNUInstallDirs)
install(TARGETS VcmxTo3D
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Finalize the executable for Qt 6
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(VcmxTo3D)
endif()
