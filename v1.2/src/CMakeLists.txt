set(SRC_LIST
    Address.cpp Address.h
    EchoServer.cpp
    Sockets.cpp Sockets.h
    EventLoop.cpp EventLoop.h
    Epoller.cpp Epoller.h
    Channel.cpp Channel.h
    )

add_executable(EchoServer ${SRC_LIST})