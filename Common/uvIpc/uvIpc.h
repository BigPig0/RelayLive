#ifndef _UV_IPC_H_
#define _UV_IPC_H_

#if ( defined _WIN32 )
#ifndef _UV_IPC_API
#ifdef UV_IPC_EXPORT
#define _UV_IPC_API		_declspec(dllexport)
#else
#define _UV_IPC_API		extern
#endif
#endif
#elif ( defined __unix ) || ( defined __linux__ )
#ifndef _UV_IPC_API
#define _UV_IPC_API        extern
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _uv_ipc_handle_ uv_ipc_handle_t;

/** 服务端接口 */

/**
 * 注册服务端
 * @param h 输出句柄
 * @param ipc ipc名字，一个合法的字符串
 * @param uv uv_loop_t指针,外部指定或NULL表示内部创建
 * @note 外部指定uv_loop_t时，需要外部处理线程间的关系
 */
_UV_IPC_API int uv_ipc_server(uv_ipc_handle_t** h, char* ipc, void* uv);

/** 客户端接口 */

/**
 * 客户端接收到消息回调
 * @param h 输出句柄
 * @param user 用户自定义数据
 * @param name 消息发送者名称,NULL表示是服务端发送的信息
 * @param msg 消息名称
 * @param data 消息内容
 * @param len 消息内容长度
 */
typedef void (*uv_ipc_recv_cb)(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len);

/**
 * 注册客户端
 * @param h 输出句柄
 * @param ipc ipc名字，一个合法的字符串
 * @param uv uv_loop_t指针,外部指定或NULL表示内部创建
 * @param name 客户端名字，不包含','的字符串
 * @param cb 接收到消息的回调方法
 * @param user 用户自定义数据
 * @note 外部指定uv_loop_t时，需要外部处理线程间的关系
 */
_UV_IPC_API int uv_ipc_client(uv_ipc_handle_t** h, char* ipc, void* uv, char* name, uv_ipc_recv_cb cb, void* user);

/**
 * 发送消息到其他客户端
 * @param h 环境句柄
 * @param names 其他客户端的名字，多个接受者用','隔开
 * @param msg 消息名称
 * @param data 消息内容，可以为NULL
 * @param len 消息内容长度
 */
_UV_IPC_API int uv_ipc_send(uv_ipc_handle_t* h, char* names, char* msg, char* data, int len);

/** 通用 */

_UV_IPC_API void uv_ipc_close(uv_ipc_handle_t* h);

_UV_IPC_API const char* uv_ipc_strerr(int status);

#ifdef __cplusplus
}
#endif
#endif