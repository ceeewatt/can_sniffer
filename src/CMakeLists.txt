set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
    main.cpp
    can_sniffer.cpp
    can_sniffer.hpp
    model.cpp
    model.hpp
    model_buffer.hpp
)

set(QT_PACKAGES
    Core
    Widgets
    SerialBus
)
list(TRANSFORM QT_PACKAGES PREPEND "Qt6::" OUTPUT_VARIABLE QT_LIBS)

find_package(Qt6 REQUIRED COMPONENTS ${QT_PACKAGES})

qt_add_executable(can_sniffer
    ${SOURCES}
)

target_compile_options(can_sniffer PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wpedantic
    -Wfatal-errors
)

target_link_libraries(can_sniffer PRIVATE ${QT_LIBS})
target_include_directories(can_sniffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(can_sniffer PRIVATE MiniJ1939::mini_j1939_lib)
