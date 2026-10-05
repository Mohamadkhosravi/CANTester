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

# 1. پیدا کردن خودکار تمام فایل‌های داخل پوشه‌های HighSpeed و LowSpeed
file(GLOB HIGH_SPEED_SOURCES "CANDatabase/HighSpeed/*.h" "CANDatabase/HighSpeed/*.cpp")
file(GLOB LOW_SPEED_SOURCES "CANDatabase/LowSpeed/*.h" "CANDatabase/LowSpeed/*.cpp")
file(GLOB CAN_DB_COMMON "CANDatabase/*.h" "CANDatabase/*.cpp")

# 2. فایل‌های اصلی ریشه پروژه
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ${CAN_DB_COMMON}
    ${HIGH_SPEED_SOURCES}
    ${LOW_SPEED_SOURCES}
)

# 3. تعریف Executable با تمام فایل‌ها
qt_add_executable(CAN_Test
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

# 4. معرفی مسیرهای Include تا هدرها بدون نیاز به نوشتن مسیر نسبی در #include شناخته شوند
target_include_directories(CAN_Test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/CANDatabase
    ${CMAKE_CURRENT_SOURCE_DIR}/CANDatabase/HighSpeed
    ${CMAKE_CURRENT_SOURCE_DIR}/CANDatabase/LowSpeed
)

# 5. لینک کردن کتابخانه‌های Qt
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