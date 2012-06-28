cmake_minimum_required(VERSION 2.6)

PROJECT(InteractivePatchProjection)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

include_directories(/media/portable/src/Eigen)

# Where to copy executables when 'make install' is run
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) #fix the dynamic linking error (RPATH) that would occur without this

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Submodules
# add_subdirectory(VTKHelpers)
add_subdirectory(ITKVTKHelpers)
add_subdirectory(ITKQtHelpers)
add_subdirectory(ITKVTKCamera)
add_subdirectory(Layer)
add_subdirectory(PatchProjection)
add_subdirectory(vtkPointSelectionStyle)

add_subdirectory(Widgets)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Widgets ${CMAKE_CURRENT_BINARY_DIR}/Widgets)

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

if(${VTK_MAJOR_VERSION} LESS 6)
  message(FATAL_ERROR "Your VTK version (${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}) is too old! At least 6.0 is required.")
endif()

FIND_PACKAGE(ITK 4 REQUIRED)
INCLUDE(${ITK_USE_FILE})

QT4_WRAP_UI(InteractivePatchProjectionWidgetUISrcs
InteractivePatchProjectionWidget.ui)
QT4_WRAP_CPP(InteractivePatchProjectionWidgetMOCSrcs
InteractivePatchProjectionWidget.h)

ADD_EXECUTABLE(InteractivePatchProjection
InteractivePatchProjectionWidget.cpp
Interactive.cpp
${InteractivePatchProjectionWidgetUISrcs} ${InteractivePatchProjectionWidgetMOCSrcs})

TARGET_LINK_LIBRARIES(InteractivePatchProjection
EigenHelpers
#QtHelpers VTKHelpers
Helpers
ITKHelpers
ITKVTKHelpers
ITKVTKCamera
#PatchClustering
Layer
libCustomWidgets
PointSelectionStyle
${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})
INSTALL( TARGETS InteractivePatchProjection RUNTIME DESTINATION ${INSTALL_DIR} )
