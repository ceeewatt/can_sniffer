set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
    qstring.cpp
)

set(QT_PACKAGES
    Core
    Widgets
)
list(TRANSFORM QT_PACKAGES PREPEND "Qt6::" OUTPUT_VARIABLE QT_LIBS)

find_package(Qt6 REQUIRED COMPONENTS ${QT_PACKAGES})

qt_add_executable(playground
    ${SOURCES}
)

target_link_libraries(playground PRIVATE ${QT_LIBS})
