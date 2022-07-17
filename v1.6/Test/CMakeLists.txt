add_executable(EchoServerTest EchoServerTest.cpp)
target_link_libraries(EchoServerTest src)

add_executable(DiscardServerTest DiscardServerTest.cpp)
target_link_libraries(DiscardServerTest src)

add_executable(DayTimeServerTest DayTimeServerTest.cpp)
target_link_libraries(DayTimeServerTest src)

add_executable(TimerTest TimerTest.cpp)
target_link_libraries(TimerTest src)

add_executable(TimerfdTest TimerfdTest.cpp)
target_link_libraries(TimerfdTest src)
