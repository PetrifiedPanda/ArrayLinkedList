cmake_minimum_required(VERSION 3.10.2)

set(This ArrayLinkedListTest)

set(Sources
    TestMain.cpp
    ArrayLinkedListTest.cpp
)

add_executable(${This} ${Sources})
target_link_libraries(${This}
    gtest_main
    ArrayLinkedList
)

add_test(
    NAME ${This}
    COMMAND ${This}
)