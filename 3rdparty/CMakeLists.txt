find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport)
qt_add_library(qcustomplot SHARED
	qcustomplot/qcustomplot.cpp qcustomplot/qcustomplot.h
)
target_compile_definitions(qcustomplot
PRIVATE
	QCUSTOMPLOT_COMPILE_LIBRARY
INTERFACE
	QCUSTOMPLOT_USE_LIBRARY
)
target_include_directories(qcustomplot INTERFACE qcustomplot)
target_link_libraries(qcustomplot PRIVATE
	Qt::Widgets
	Qt::PrintSupport
)
