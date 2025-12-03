find_package(Threads)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
