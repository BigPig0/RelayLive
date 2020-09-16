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

/** ����˽ӿ� */

/**
 * ע������
 * @param h ������
 * @param ipc ipc���֣�һ���Ϸ����ַ���
 * @param uv uv_loop_tָ��,�ⲿָ����NULL��ʾ�ڲ�����
 * @note �ⲿָ��uv_loop_tʱ����Ҫ�ⲿ�����̼߳�Ĺ�ϵ
 */
_UV_IPC_API int uv_ipc_server(uv_ipc_handle_t** h, char* ipc, void* uv);

/** �ͻ��˽ӿ� */

/**
 * �ͻ��˽��յ���Ϣ�ص�
 * @param h ������
 * @param user �û��Զ�������
 * @param name ��Ϣ����������,NULL��ʾ�Ƿ���˷��͵���Ϣ
 * @param msg ��Ϣ����
 * @param data ��Ϣ����
 * @param len ��Ϣ���ݳ���
 */
typedef void (*uv_ipc_recv_cb)(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len);

/**
 * ע��ͻ���
 * @param h ������
 * @param ipc ipc���֣�һ���Ϸ����ַ���
 * @param uv uv_loop_tָ��,�ⲿָ����NULL��ʾ�ڲ�����
 * @param name �ͻ������֣�������','���ַ���
 * @param cb ���յ���Ϣ�Ļص�����
 * @param user �û��Զ�������
 * @note �ⲿָ��uv_loop_tʱ����Ҫ�ⲿ�����̼߳�Ĺ�ϵ
 */
_UV_IPC_API int uv_ipc_client(uv_ipc_handle_t** h, char* ipc, void* uv, char* name, uv_ipc_recv_cb cb, void* user);

/**
 * ������Ϣ�������ͻ���
 * @param h �������
 * @param names �����ͻ��˵����֣������������','����
 * @param msg ��Ϣ����
 * @param data ��Ϣ���ݣ�����ΪNULL
 * @param len ��Ϣ���ݳ���
 */
_UV_IPC_API int uv_ipc_send(uv_ipc_handle_t* h, char* names, char* msg, char* data, int len);

/** ͨ�� */

_UV_IPC_API void uv_ipc_close(uv_ipc_handle_t* h);

_UV_IPC_API const char* uv_ipc_strerr(int status);

#ifdef __cplusplus
}
#endif
#endif