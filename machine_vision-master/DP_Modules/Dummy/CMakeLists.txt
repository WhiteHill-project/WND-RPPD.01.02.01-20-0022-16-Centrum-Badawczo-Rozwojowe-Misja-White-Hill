set(MODULE_SRC
    main.cpp
)

add_executable(DP_Dummy ${MODULE_SRC})
target_link_libraries(DP_Dummy TestSetConnector)
