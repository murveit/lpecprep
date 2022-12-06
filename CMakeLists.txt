PROJECT(LPecPrep C CXX)
cmake_minimum_required(VERSION 3.0)

#cmake_policy(SET CMP0003 NEW)

# Timestamp build
string(TIMESTAMP LPecPrep_BUILD_TS UTC)

# LPecPrep Version 2.4
set (LPecPrep_VERSION_MAJOR 1)
set (LPecPrep_VERSION_MINOR 0)

set (LPecPrep_SOVERSION "${LPecPrep_VERSION_MAJOR}")
set (LPecPrep_VERSION ${LPecPrep_VERSION_MAJOR}.${LPecPrep_VERSION_MINOR})
set (LPecPrep_VERSION_STRING "${LPecPrep_VERSION_MAJOR}.${LPecPrep_VERSION_MINOR}")

# Exports
set(TARGET_EXPORT_NAME ${PROJECT_NAME}Targets)
set(TARGET_EXPORT_CONFIG ${PROJECT_NAME}Config)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
endif(APPLE)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")
include(GNUInstallDirs)

include_directories( "${CMAKE_CURRENT_BINARY_DIR}")
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/lpecprep")
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot-source")

find_package(GSL REQUIRED)
find_package(Qt5 5.4 REQUIRED COMPONENTS Gui Widgets Core PrintSupport Concurrent Network)

if (GSL_FOUND)
  include_directories(${GSL_INCLUDE_DIR})
endif (GSL_FOUND)

set(LPecPrep_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot-source/qcustomplot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lpecprep/fftutil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lpecprep/phdconvert.cpp
   )

set(ALL_SRCS
    ${LPecPrep_SRCS}
    )
add_library(lpecprep SHARED ${ALL_SRCS})

target_link_libraries(lpecprep
    ${GSL_LIBRARIES}
    Qt5::Core
    Qt5::Network
    Qt5::Widgets
    Qt5::Concurrent
    Qt5::PrintSupport    
    )

set_target_properties(lpecprep PROPERTIES VERSION ${LPecPrep_VERSION_STRING} SOVERSION ${LPecPrep_SOVERSION} OUTPUT_NAME lpecprep)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lpecprep.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/lpecprep.pc @ONLY)

set(LPecPrepMain_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/lpecprep/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lpecprep/mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lpecprep/resources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/lpecprep/analysis.cpp
    )

qt5_wrap_ui(LPecPrepui_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/lpecprep/mainwindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/lpecprep/analysis.ui
    )

add_executable(LPecPrep ${LPecPrepMain_SRCS} ${LPecPrepui_SRCS} ${LPecPrepApp_ICON})

target_link_libraries(LPecPrep
    lpecprep
    ${GSL_LIBRARIES}
    Qt5::Gui
    Qt5::Widgets
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    )

  
set(Tester_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/lpecprep/main_tester.cpp
    )

add_executable(Tester ${Tester_SRCS} ${LPecPrepApp_ICON})

target_link_libraries(Tester
    lpecprep
    ${GSL_LIBRARIES}
    Qt5::Gui
    Qt5::Widgets
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    )

  
