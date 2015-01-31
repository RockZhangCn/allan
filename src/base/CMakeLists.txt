file(GLOB base_SRCS
    "*.cc")

add_library(choudan_base ${base_SRCS})
target_link_libraries(choudan_base pthread)

install(TARGETS choudan_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/choudan/base)
