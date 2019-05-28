# RelayLive
## 地址: 
*  https://github.com/BigPig0/RelayLive.git
*  https://gitee.com/ztwlla/RelayLive.git

## 说明:
* sip_server: sip服务器，用来与下级平台交互
* relay_live: http和直播服务器
* ipc_server: 进程间通讯的工具，sip_server和relay_live通过改程序通讯。因此需要先启动ipc，再启动另外两个程序。

## 第三方:
* libwebsockets: https://github.com/warmcat/libwebsockets.git
* libuv: https://github.com/libuv/libuv.git
* luapp: https://github.com/ToyAuthor/luapp.git