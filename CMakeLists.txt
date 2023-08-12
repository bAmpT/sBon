cmake_minimum_required(VERSION 3.5)

project(sBon VERSION 3.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Sql Core PrintSupport REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Sql Core PrintSupport REQUIRED)

set(SOURCE_DIR
    src
)
set(PROJECT_SOURCES
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/mainwindow.cpp
    ${SOURCE_DIR}/mainwindow.h
    ${SOURCE_DIR}/productdelegate.cpp
    ${SOURCE_DIR}/productdelegate.h
    ${SOURCE_DIR}/producttablemodel.cpp
    ${SOURCE_DIR}/producttablemodel.h
    ${SOURCE_DIR}/addproductbutton.cpp
    ${SOURCE_DIR}/addproductbutton.h
    ${SOURCE_DIR}/bontablemodel.cpp
    ${SOURCE_DIR}/bontablemodel.h
    ${SOURCE_DIR}/editdialog.cpp
    ${SOURCE_DIR}/editdialog.h
    ${SOURCE_DIR}/editdialog.ui
    ${SOURCE_DIR}/mainwindow.ui
    ${SOURCE_DIR}/editproductdialog.h
    ${SOURCE_DIR}/editproductdialog.cpp
    ${SOURCE_DIR}/flowlayout.h
    ${SOURCE_DIR}/flowlayout.cpp
    ${SOURCE_DIR}/producttabwidget.h
    ${SOURCE_DIR}/producttabwidget.cpp
    include/productmodel.h
    include/initdb.h
    #Info.plist
)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(sBon
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET sBon APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(sBon SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(sBon
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(sBon PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::PrintSupport    
    Qt${QT_VERSION_MAJOR}::Core
)

set_target_properties(sBon PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER sbon.gistastic.com
    #MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
    #MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    #MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(sBon)
endif()
