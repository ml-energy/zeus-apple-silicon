set(MOCKER_DIR ${PROJECT_SOURCE_DIR}/tests/mocker)

set(
    MOCKER_SOURCES
    ${PROJECT_SOURCE_DIR}/tests/mocker/ioreport_mocker.cpp
)

add_subdirectory(tests_cpp)
add_subdirectory(mocker)
