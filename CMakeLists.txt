cmake_minimum_required(VERSION 3.16)
set(QT_CREATOR_SKIP_PACKAGE_MANAGER ON CACHE BOOL "" FORCE)
project(CAN_Test VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        BCM_4.h
        BCM_5.h
        HCU_1.h
        HCU_10.h
        EMS_3.h
        HVAC_1.h
        KT08Database.cpp
        KT08Database.h      
        ICanMessage.h
        ICANDatabase.h

)
qt_add_executable(CAN_Test
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_include_directories(CAN_Test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CAN_Test PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

set_target_properties(CAN_Test PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CAN_Test)
endif()