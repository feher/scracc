include_directories (${Boost_INCLUDE_DIR})
add_definitions ( "-DHAS_BOOST" )

add_library (scracc SHARED libscracc)
target_link_libraries(scracc ${Boost_LIBRARIES} cryptopp)

install (TARGETS scracc
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PUBLIC_HEADER DESTINATION include/scracc
         COMPONENT libscracc)
install (FILES libscracc.h
         DESTINATION include/scracc
         COMPONENT libscracc)
