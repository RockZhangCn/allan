file(GLOB adns_SRCS
    "*.cc")

add_executable(adns_client ${adns_SRCS})
target_link_libraries(adns_client choudan_base adns)
