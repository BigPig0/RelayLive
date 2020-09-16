#include "stdafx.h"
#include "H264Worker.h"
#include "SipInstance.h"
#include "libLive.h"
#include "StreamManager.h"

static void destroy_h264nalu(void *_msg)
{
    CH264Nalu *msg = (CH264Nalu*)_msg;
    free(msg->pBuff);
    msg->pBuff = NULL;
    msg->nLen = 0;
}

CH264Worker::CH264Worker(string strPlatformCode, string strDevCode)
    : m_strPlatformCode(strPlatformCode)
    , m_strDevCode(strDevCode)
{
    m_sRing = lws_ring_create(sizeof(CH264Nalu), 100, destroy_h264nalu);
}


CH264Worker::~CH264Worker(void)
{
    if (!SipInstance::StopPlay(m_strPlatformCode, m_strDevCode))
    {
        Log::error("stop play failed");
    }
    lws_ring_destroy(m_sRing);
}


void CH264Worker::AddTag(AutoMemoryPtr buff)
{
    //Log::debug("type:%d, size:%d",eType,nBuffSize);
        CH264Nalu newTag;
        newTag.pBuff = (char*)malloc(buff->nLen);
        CHECK_POINT_VOID(newTag.pBuff);
        newTag.nLen = buff->nLen;
        memcpy(newTag.pBuff, buff->pBuff, buff->nLen);

        int n = (int)lws_ring_get_count_free_elements(m_sRing);
        if (!n) {
            cull_lagging_clients();
            n = (int)lws_ring_get_count_free_elements(m_sRing);
        }
        Log::debug("LWS_CALLBACK_RECEIVE: free space %d\n", n);
        if (!n)
            return;

        // 将数据保存在ring buff
        if (!lws_ring_insert(m_sRing, &newTag, 1)) {
            destroy_h264nalu(&newTag);
            lwsl_user("dropping!\n");
            return;
        }

        //向所有播放链接发送数据
        lws_start_foreach_llp(pss_http_ws_live **, ppss, m_sPssList) {
            lws_callback_on_writable((*ppss)->wsi);
        } lws_end_foreach_llp(ppss, pss_next);
        
}

bool CH264Worker::AddConnect(void* pHttp)
{
    pss_http_ws_live* pss = (pss_http_ws_live*)pHttp;
    pss->pss_next = m_sPssList;
    m_sPssList = pss;
    pss->tail = lws_ring_get_oldest_tail(m_sRing);
    return true;
}

bool CH264Worker::DelConnect(void* pHttp)
{
    pss_http_ws_live *pss = (pss_http_ws_live*)pHttp;
    lws_ll_fwd_remove(pss_http_ws_live, pss_next, pss, m_sPssList);

    if (m_sPssList == nullptr)
    {
		std::thread t([&](){
			Sleep(20000);
			if (m_sPssList == nullptr) {
				CH264Dock* pFlv = CH264Dock::GetInstance();
				CHECK_POINT_VOID(pFlv)
				pFlv->DelWorker(m_strPlatformCode, m_strDevCode);
			}
		});
		t.detach();
    }
    return true;
}

void CH264Worker::cull_lagging_clients()
{
	uint32_t oldest_tail = lws_ring_get_oldest_tail(m_sRing);
	pss_http_ws_live *old_pss = NULL;
	int most = 0, before = lws_ring_get_count_waiting_elements(m_sRing, &oldest_tail), m;


	lws_start_foreach_llp_safe(pss_http_ws_live **, ppss, m_sPssList, pss_next) {

		if ((*ppss)->tail == oldest_tail) {
			old_pss = *ppss;

			lwsl_user("Killing lagging client %p\n", (*ppss)->wsi);

			lws_set_timeout((*ppss)->wsi, PENDING_TIMEOUT_LAGGING, LWS_TO_KILL_ASYNC);

			(*ppss)->culled = 1;

			lws_ll_fwd_remove(pss_http_ws_live, pss_next, (*ppss), m_sPssList);

			continue;

		} else {
			m = lws_ring_get_count_waiting_elements(m_sRing, &((*ppss)->tail));
			if (m > most)
				most = m;
		}

	} lws_end_foreach_llp_safe(ppss);

	/* it would mean we lost track of oldest... but Coverity insists */
	if (!old_pss)
		return;

	/*
	 * Let's recover (ie, free up) all the ring slots between the
	 * original oldest's last one and the "worst" survivor.
	 */

	lws_ring_consume_and_update_oldest_tail(m_sRing,
		pss_http_ws_live, &old_pss->tail, before - most,
		m_sPssList, tail, pss_next);

	lwsl_user("%s: shrunk ring from %d to %d\n", __func__, before, most);
}


//////////////////////////////////////////////////////////////////////////

CH264Dock::CH264Dock()
{

}

CH264Dock::~CH264Dock()
{

}

CH264Worker* CH264Dock::CreateWorker(string strPlatformCode, string strDevCode)
{
    Log::debug("CreateWorker begin");
    string strKey = strPlatformCode + "," + strDevCode;

    CH264Worker* pNew = new CH264Worker(strPlatformCode, strDevCode);
    {
        MutexLock lock(&m_cs);
        m_workerMap.insert(make_pair(strKey, pNew));
        Log::debug("new h264 ok");
    }

    if(!SipInstance::RealPlay(strPlatformCode, strDevCode))
    {
        DelWorker(strPlatformCode, strDevCode);
        Log::error("play failed %s,%s",strPlatformCode.c_str(), strDevCode.c_str());
        return nullptr;
    }
    Log::debug("RealPlay ok, key:%s",strKey.c_str());

    IStreamManager* pStream = IStreamManager::GetInstance();
    pStream->SetCallbackH264(strPlatformCode, strDevCode, [pNew,strPlatformCode,strDevCode](AutoMemoryPtr buff){
        pNew->AddTag(buff);
    });

    return pNew;
}

CH264Worker* CH264Dock::GetWorker(string strPlatformCode, string strDevCode)
{
    Log::debug("GetWorker begin");
    string strKey = strPlatformCode + "," + strDevCode;

    MutexLock lock(&m_cs);
    auto itFind = m_workerMap.find(strKey);
    if (itFind != m_workerMap.end())
    {
        // 已经存在
        return itFind->second;
    }

    Log::error("GetWorker failed, key:%s",strKey.c_str());
    return nullptr;
}

bool CH264Dock::DelWorker(string strPlatformCode, string strDevCode)
{
    MutexLock lock(&m_cs);
    string strKey = strPlatformCode + "," + strDevCode;
    auto itFind = m_workerMap.find(strKey);
    if (itFind != m_workerMap.end())
    {
        SAFE_DELETE(itFind->second);
        m_workerMap.erase(itFind);
        return true;
    }

    Log::error("dosn't find h264 worker object");
    return false;
}