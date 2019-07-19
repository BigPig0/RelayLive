
namespace LiveClient
{
namespace LiveIpc
{
    extern void Init();

    extern int RealPlay(string dev_code, string rtp_ip, int rtp_port);

    extern int StopPlay(int rtp_port);

    extern int GetDevList();

    extern int QueryDirtionary();

	extern int DeviceControl(string strDev,
        int nInOut = 0, int nUpDown = 0, int nLeftRight = 0);
}
}