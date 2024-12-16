Many OSs provide a more efficient interface for event notification: POSIX provides poll, Linux has epoll, FreeBSD and macOS use kqueue, Windows has IOCP, 
Solaris has /dev/poll, and so on. These basic primitives allow us to build a nonblocking event loop that simply checks incoming packets, 
reads socket messages, and responds as needed.



NOTE A lot of popular core libraries and frameworks have been built on the ideas we outline here. 
Libevent is a widely used, long-standing cross-platform event library; libuv (an abstraction layer on top of libeio, libev, c-ares, and iocp)
implements low-level I/O in Node.js, Java NIO, NGINX, and Vert.x using nonblocking models with an event loop implementation to achieve a high level of concurrency.
