Many OSs provide a more efficient interface for event notification: POSIX provides poll, Linux has epoll, FreeBSD and macOS use kqueue, Windows has IOCP, 
Solaris has /dev/poll, and so on. These basic primitives allow us to build a nonblocking event loop that simply checks incoming packets, 
reads socket messages, and responds as needed.
