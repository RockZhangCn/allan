file(GLOB block_queue_SRCS
    "*.cc")

add_executable(test_block_queue ${block_queue_SRCS})
target_link_libraries(test_block_queue choudan_base adns)
