# Networking Server step by step
This project is used for recording the development process or notes of a server (net lib).
- [Networking Server step by step](#networking-server-step-by-step)
- [Version 1.0](#version-10)
- [Version 1.1](#version-11)
- [Version 1.2](#version-12)
- [Version 1.3](#version-13)
- [Version 1.4](#version-14)
- [ChangeLog](#changelog)
  - [Version 1.0](#version-10-1)
  - [Version 1.1](#version-11-1)
  - [Version 1.2](#version-12-1)
  - [Version 1.3](#version-13-1)
  - [Version 1.4](#version-14-1)

# [Version 1.0](doc/Notes_for_Version_1.md)
It's a simple echo server with I/O multiplexing.

# [Version 1.1](doc/Notes_for_Version_1_1.md)
It's a simple echo server with based-object programming and I/O multiplexing.

# [Version 1.2](doc/Notes_for_Version_1_2.md)
It's a simple echo server with based-object programming and I/O multiplexing with init Reactor.

# [Version 1.3](doc/Notes_for_Version_1_3.md)
It's a simple daytime server with based-object programming and I/O multiplexing with init Reactor.

# [Version 1.4](doc/Notes_for_Version_1_4.md)
It's a discard server with based-object programming and I/O multiplexing with init single-thread Reactor but it cannot shutdown connections normally.

# ChangeLog
2022.05.13
* Starts Working.
## Version 1.0
2022.05.13
* EchoServer works.
* Notes for ver1.0 finished.

## Version 1.1
2022.05.21
* Address works.

2022.05.22
* Epoller works.
* EventLoop works.
* Sockets works.

2022.05.23
* Notes for ver1.1 finished.

## Version 1.2
2022.06.02
* Channel works.
* Fixed bug.

2022.06.08
* Notes for ver1.2 finished.

## Version 1.3
2022.06.11
* Thread works.

2022.06.12
* Acceptor works.
* Reprogramming EventLoop, Channel, Epoller.

2022.06.14
* Notes for ver1.3 finished.

## Version 1.4
2022.06.19
* TcpConnection works.
* TcpServer works.

