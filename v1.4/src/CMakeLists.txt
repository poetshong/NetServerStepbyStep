set(SRC_LIST
    Address.cpp Address.h
    Sockets.cpp Sockets.h
    EventLoop.cpp EventLoop.h
    Epoller.cpp Epoller.h
    Channel.cpp Channel.h
    Acceptor.cpp Acceptor.h
    TcpServer.cpp TcpServer.h
    TcpConnection.cpp TcpConnection.h
    Callbacks.h
    )

add_library(src STATIC ${SRC_LIST})
target_link_libraries(src pthread)

# install(TARGETS src DESTINATION lib)
# set(HEADERS
#     Address.h
#     Sockets.h
#     EventLoop.h
#     Epoller.h
#     Channel.h
#     # Thread.h
#     Acceptor.h
#     )

# install(FILES ${HEADERS} DESTINATION include)
