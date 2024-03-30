cmake_minimum_required(VERSION 3.16)
project(MyWidgets)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Core Widgets Concurrent)
find_package(qcustomplot CONFIG)
qt_add_executable(${PROJECT_NAME}
	wgt/ClickableLabel.hpp
	wgt/EitherCheckBox.cpp
	wgt/Plot/Plot.cpp
	wgt/Plot/PlotSettingDialog.cpp
	main.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE wgt)
target_link_libraries(${PROJECT_NAME} PRIVATE
	Qt::Widgets
	Qt::Concurrent
	qcustomplot
)
