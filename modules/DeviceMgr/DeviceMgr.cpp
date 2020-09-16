#include "util.h"
#include "luapp.hpp"
#include <thread>
#include "DeviceMgr.h"
#include "DataBase.h"

namespace DeviceMgr
{
    PlatFormInfo         _platform;   //< ƽ̨��Ϣ
    map<string,DevInfo*> _mapDevInfo; //< �豸��Ϣ
    CriticalSection      _cs;
    bool                 _bExpireRun; //< ��ʱ�߳��Ƿ���
    CDataBase            _db;         //< ���ݿ����

	static bool checkStringDouble(string val1, string val2) {
		double d1 = val1.empty() ? 0 : atof(val1.c_str());
		double d2 = val2.empty() ? 0 : atof(val2.c_str());
		if (abs(d1-d2) <= 0.000001)
			return true;
		return false;
	}

    static void StartExpireTimer()
    {
        _bExpireRun = true;
        std::thread t1([](){
            Log::debug("ExpireTimer thread ID : %d", GetCurrentThreadId());
            while(_bExpireRun)
            {
                _platform.nExpire -= 10;
                Sleep(10000);
            }
        });
        t1.detach();
    }

    /**
    * �����ݿ��ȡ�豸
    */
    static void getDeviceFromDB()
    {
        vector<DevInfo*> vecDev = _db.GetDevInfo();
        AddDevice(vecDev, false);
    }

    bool Init()
    {
        _platform.strDevCode = Settings::getValue("PlatFormInfo","Code");
        _platform.strAddrIP = Settings::getValue("PlatFormInfo","IP");
        _platform.strAddrPort = Settings::getValue("PlatFormInfo","Port");
        _platform.strStatus = "1";
        _platform.nExpire = 3600;
        _bExpireRun = true;
        StartExpireTimer();

        _db.Init();
        getDeviceFromDB();
        return true;
    }

    bool Cleanup()
    {
        _bExpireRun = false;
        return true;
    }

    bool RegistPlatform(PlatFormInfo platform)
    {
        Log::debug("update %s expire %d->%d"
            , platform.strDevCode.c_str()
            , _platform.nExpire
            , platform.nExpire);
        _platform.strDevCode = platform.strDevCode;
        _platform.strAddrIP = platform.strAddrIP;
        _platform.strAddrPort = platform.strAddrPort;
        _platform.strStatus = platform.strStatus;
        _platform.nExpire = platform.nExpire;

        return true;
    }

    bool KeepAlivePlatform()
    {
        _platform.nExpire = 120;
        return true;
    }

    bool IsPlatformLive()
    {
        return _platform.nExpire>0?true:false;
    }

    PlatFormInfo* GetPlatformInfo()
    {
        return &_platform;
    }

    bool AddDevice(vector<DevInfo*> vecDevInfo, bool bUpdate)
    {
        MutexLock lock(&_cs);
        size_t nNum = vecDevInfo.size();
        for (size_t i=0; i<nNum; ++i)
        {
            DevInfo* pDev = vecDevInfo[i];
			if(bUpdate)
				_db.UpdateDevPos(pDev);
            //
            auto findDev = _mapDevInfo.find(vecDevInfo[i]->strDevID);
            if (findDev == _mapDevInfo.end())
            {
                // ��һ����Ӹ��豸
                _mapDevInfo.insert(make_pair(pDev->strDevID,pDev));

                //Ҳ����Ҫ����Ϣ���뵽���ݿ�
                if(bUpdate)
                    _db.InsertDev(pDev);
            }
            else
            {
                // �������ӣ�����Ҫ�ظ�����
                //���ݿ�����Ϣ����
				if(!pDev->strStatus.empty() && pDev->strStatus != findDev->second->strStatus) {
					findDev->second->strStatus = pDev->strStatus;
					if(bUpdate)
						_db.UpdateStatus(pDev->strDevID, pDev->strStatus=="ON"?true:false);
				}
				if ( (!pDev->strLongitude.empty() || !pDev->strLatitude.empty()) 
					&& ( !checkStringDouble(pDev->strLongitude, findDev->second->strLongitude)
					  || !checkStringDouble(pDev->strLatitude, findDev->second->strLatitude) ) )
				{
					findDev->second->strLongitude = pDev->strLongitude;
					findDev->second->strLatitude = pDev->strLatitude;
					if(bUpdate)
						_db.UpdatePos(pDev->strDevID, pDev->strLatitude, pDev->strLongitude);
				}
				if(!pDev->strName.empty() && pDev->strName != findDev->second->strName){
					findDev->second->strName = pDev->strName;
				}
				if(!pDev->strAddress.empty() && pDev->strAddress != findDev->second->strAddress){
					findDev->second->strAddress = pDev->strAddress;
				}
				if(!pDev->strModel.empty() && pDev->strModel != findDev->second->strModel){
					findDev->second->strModel = pDev->strModel;
				}
				if(!pDev->strModel.empty() && pDev->strModel != findDev->second->strModel){
					findDev->second->strModel = pDev->strModel;
				}

                delete pDev;
            }
        }

        return true;
    }

    bool UpdateDevice(DevInfo* pDev)
    {
        MutexLock lock(&_cs);
		_db.UpdateDevPos(pDev);
		auto findDev = _mapDevInfo.find(pDev->strDevID);
		if (findDev == _mapDevInfo.end())
		{
			// ��һ����Ӹ��豸
			//_mapDevInfo.insert(make_pair(pDev->strDevID,pDev));

			//����Ϣ���뵽���ݿ�
			//_db.InsertDev(pDev);
		}
		else
		{
			if (!pDev->strStatus.empty() && pDev->strStatus != findDev->second->strStatus)
			{
				findDev->second->strStatus = pDev->strStatus;
				_db.UpdateStatus(pDev->strDevID, pDev->strStatus=="ON"?true:false);
			}
			if ( (!pDev->strLongitude.empty() || !pDev->strLatitude.empty()) 
				&& ( !checkStringDouble(pDev->strLongitude, findDev->second->strLongitude)
				  || !checkStringDouble(pDev->strLatitude, findDev->second->strLatitude) ) )
			{
				findDev->second->strLongitude = pDev->strLongitude;
				findDev->second->strLatitude = pDev->strLatitude;
				_db.UpdatePos(pDev->strDevID, pDev->strLatitude, pDev->strLongitude);
			}
			delete pDev;
		}
        return true;
    }

    vector<DevInfo*> GetDeviceInfo()
    {
        MutexLock lock(&_cs);
        vector<DevInfo*> vecRet;
        auto it = _mapDevInfo.begin();
        auto end = _mapDevInfo.end();
        for (; it!=end; ++it)
        {
            vecRet.push_back(it->second);
        }
        return vecRet;
    }

    DevInfo* GetDeviceInfo(string strDevCode)
    {
        MutexLock lock(&_cs);
        auto findDev = _mapDevInfo.find(strDevCode);
        if (findDev != _mapDevInfo.end())
        {
            return findDev->second;
        }

        return nullptr;
    }

    bool CleanPlatform()
    {
        MutexLock lock(&_cs);
        for (auto& itDev:_mapDevInfo)
        {
            SAFE_DELETE(itDev.second);
        }
        _mapDevInfo.clear();

        _db.CleanDev();

        return true;
    }
}