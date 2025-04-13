set(MOCKER_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/tests/mocker)

set(
    MOCKER_SOURCES
    ${PROJECT_SOURCE_DIR}/tests/mocker/mock_ioreport.cpp
)

add_subdirectory(tests_cpp)
add_subdirectory(mocker)
