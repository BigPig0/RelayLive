
namespace LiveClient
{
namespace LiveIpc
{
    extern void Init();

    extern int RealPlay(string dev_code, string rtp_ip, int rtp_port);

    extern int StopPlay(string dev_code);

    extern int GetDevList();

    extern int QueryDirtionary();
}
}