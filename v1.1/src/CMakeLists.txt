set(SRC_LIST
    Address.cpp Address.h
    EchoServer.cpp
    Sockets.cpp Sockets.h
    EventLoop.cpp EventLoop.h
    Epoller.cpp Epoller.h
    )

add_executable(EchoServer ${SRC_LIST})