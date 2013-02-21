include_directories(${CMAKE_SOURCE_DIR}/libscracc)
link_directories(${CMAKE_BINARY_DIR}/libscracc)

add_executable(scracc-exe scracc)

target_link_libraries(scracc-exe scracc ${Boost_LIBRARIES} cryptopp)

install (PROGRAMS ${CMAKE_BINARY_DIR}/scracc/scracc-exe DESTINATION bin RENAME scracc)

