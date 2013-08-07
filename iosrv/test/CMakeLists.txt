project(utiltest)
include_directories(${util_SOURCE_DIR})
add_executable(test_ioservice test_ioservice.cpp)
target_link_libraries(test_ioservice ${Boost_LIBRARIES})
