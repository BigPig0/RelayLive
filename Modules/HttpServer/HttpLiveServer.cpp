#include "stdafx.h"
#include "libwebsockets.h"
#include "HttpLiveServer.h"
#include "HttpWorker.h"
#include "HlsWorker.h"
#include "LiveClient.h"

namespace HttpWsServer
{
    static string StartPlayLive(pss_http_ws_live *pss, string devCode, int channel)
    {
        if(pss == nullptr) {
            return g_strError_play_faild;
        }

        HandleType t = HandleType::unknown_handle;
        if(pss->media_type == media_flv) t = HandleType::flv_handle;
        else if(pss->media_type == media_mp4) t = HandleType::fmp4_handle;
        else if(pss->media_type == media_h264) t = HandleType::h264_handle;
        else if(pss->media_type == media_hls) t = HandleType::ts_handle;
        else if(pss->media_type == media_m3u8) t = HandleType::ts_handle;
        else if(pss->media_type == media_ts) t = HandleType::ts_handle;

        CHttpWorker* pWorker = GetHttpWorker(devCode, t, channel);
        if (!pWorker) {
            pWorker = CreatHttpWorker(devCode, t, channel);
        }
        if(!pWorker) {
            Log::error("CreatHttpWorker failed%s", devCode.c_str());
            return g_strError_play_faild;
        }
        pWorker->AddConnect(pss);

        pss->m_pWorker = pWorker;
        return "";
    }

    static void SendLiveFlv(pss_http_ws_live *pss) {
        CHttpWorker* pWorker = (CHttpWorker*)pss->m_pWorker;
        AV_BUFF tag = pWorker->GetVideo(&pss->tail);
        if(tag.pData == nullptr) return;

        if (!pss->m_bSendHead) {
            if(tag.eType != AV_TYPE::FLV_FRAG_KEY) // 第一个包必须是关键帧
                return;
            AV_BUFF flvheader = pWorker->GetHeader();
            if(flvheader.pData == nullptr)
                return;

            Log::debug("first flv data with header: tail:%d",pss->tail);
            int len = flvheader.nLen + tag.nLen;
            uint8_t* buff = (uint8_t *)malloc(len + LWS_PRE);
            int nPos = LWS_PRE;
            memcpy(buff + nPos, flvheader.pData, flvheader.nLen);
            nPos += flvheader.nLen;
            memcpy(buff + nPos, tag.pData + LWS_PRE, tag.nLen);
            int wlen = lws_write(pss->wsi, (uint8_t *)buff + LWS_PRE, len, LWS_WRITE_BINARY);
            pss->m_bSendHead = true;
            free(buff);
            pWorker->NextWork(pss);
        } else {
            //Log::debug(" flv data tail:%d", pss->tail);
            int wlen = lws_write(pss->wsi, (uint8_t *)tag.pData + LWS_PRE, tag.nLen, LWS_WRITE_BINARY);
            pWorker->NextWork(pss);
        }
    }

    static void SendLiveH264(pss_http_ws_live *pss) {
        AV_BUFF tag = pss->m_pWorker->GetVideo(&pss->tail);
		if(tag.pData == nullptr) return;

        Log::debug(" h264 data tail:%d", pss->tail);
        int wlen = lws_write(pss->wsi, (uint8_t *)tag.pData + LWS_PRE, tag.nLen, LWS_WRITE_BINARY);
        pss->m_pWorker->NextWork(pss);
    }

    static void SendLiveMp4(pss_http_ws_live *pss)
    {
        CHttpWorker* pWorker = (CHttpWorker*)pss->m_pWorker;
        AV_BUFF tag = pWorker->GetVideo(&pss->tail);
        if(tag.pData == nullptr) return;

        if (!pss->m_bSendHead) {
            if(tag.eType != AV_TYPE::MP4_FRAG_KEY) // 第一个包必须是关键帧
                return;
            AV_BUFF mp4_header = pWorker->GetHeader();
            if(mp4_header.pData == nullptr)
                return;

            Log::debug("first mp4 data with header: tail:%d",pss->tail);
            int len = mp4_header.nLen + tag.nLen;
            uint8_t* buff = (uint8_t *)malloc(len + LWS_PRE);
            int nPos = LWS_PRE;
            memcpy(buff + nPos, mp4_header.pData, mp4_header.nLen);
            nPos += mp4_header.nLen;
            memcpy(buff + nPos, tag.pData + LWS_PRE, tag.nLen);
            int wlen = lws_write(pss->wsi, (uint8_t *)buff + LWS_PRE, len, LWS_WRITE_BINARY);
            pss->m_bSendHead = true;
            free(buff);
            pWorker->NextWork(pss);
        } else {
            Log::debug(" mp4 data tail:%d", pss->tail);
            int wlen = lws_write(pss->wsi, (uint8_t *)tag.pData + LWS_PRE, tag.nLen, LWS_WRITE_BINARY);
            pWorker->NextWork(pss);
        }
    }

    static string StartPlayHls(pss_http_ws_live *pss, string devCode){
        if(pss == nullptr) {
            return g_strError_play_faild;
        }
        CHlsWorker* pWorker = GetHlsWorker(devCode);
        if (!pWorker) {
            pWorker = CreatHlsWorker(devCode);
        }
        if(!pWorker) {
            Log::error("CreatHlsWorker failed%s", devCode.c_str());
            return g_strError_play_faild;
        }
        pss->m_pWorker = (CHttpWorker*)pWorker;
        return "";
    }

    static bool SendLiveM3u8(pss_http_ws_live *pss){
        CHlsWorker* pWorker = (CHlsWorker*)pss->m_pWorker;
        AV_BUFF tag = pWorker->GetHeader();
        if(tag.pData == nullptr) {
			pWorker->AddConnect(pss);
            return false;
		}

        int wlen = lws_write(pss->wsi, (uint8_t *)tag.pData, tag.nLen, LWS_WRITE_TEXT);
        return true;
    }

    static bool SendLiveTs(pss_http_ws_live *pss){
        CHlsWorker* pWorker = (CHlsWorker*)pss->m_pWorker;
        string strPath = pss->path;

        size_t pos = strPath.rfind(".");
        size_t pos2 = strPath.rfind("/");
        if(pos == string::npos || pos2 == string::npos) {
            return false;
        }
		Log::debug("pos   %lld  %lld", pos, pos2);

        string devcode = strPath.substr(1, pos2 -1);
        string tsid = strPath.substr(pos2+1, pos - pos2 - 1);
        uint64_t id = _atoi64(tsid.c_str());

        Log::debug("get video %lld", id);
        AV_BUFF tag = pWorker->GetVideo(id);
        if(tag.pData == nullptr) {
            return false;
        }

		Log::debug("send ts");
        int wlen = lws_write(pss->wsi, (uint8_t *)tag.pData, tag.nLen, LWS_WRITE_BINARY);
        return true;
    }

    int callback_live_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
    {
        pss_http_ws_live *pss = (pss_http_ws_live*)user;

        switch (reason) {
        case LWS_CALLBACK_HTTP: 
            {
                uint8_t buf[LWS_PRE + 2048],
                    *start = &buf[LWS_PRE],
                    *p = start,
                    *end = &buf[sizeof(buf) - LWS_PRE - 1];

                lws_snprintf(pss->path, sizeof(pss->path), "%s", (const char *)in);
                Log::debug("new request: %s", pss->path);

                pss->wsi = wsi;
                pss->pss_next = nullptr;
                pss->isWs = false;
                pss->m_bSendHead = false;
                pss->m_pWorker = nullptr;
                lws_get_peer_addresses(wsi, lws_get_socket_fd(wsi),
                    pss->clientName, sizeof(pss->clientName),
                    pss->clientIP, sizeof(pss->clientIP));


                string strErrInfo;
                string strPath = pss->path;
                // http://IP:port/live/type/channel/code
                // type: flv、mp4、h264、hls
                // channel: 0:原始码流 1:小码流
                // e.g http://localhost:80/live/flv/0/123456789
                // 这里path只有 /type/stream/code
                do{
                    char szType[10]={0}, szCode[30]={0};
                    int nChannel = 0;
                    int ret = sscanf(pss->path, "/%[a-z0-9]/%d/%[0-9a-z]", szType, &nChannel, szCode);
                    if(ret < 0) {
                        Log::error("%s error sscanf %d", pss->path, ret);
                        strErrInfo = "error request path";
                        break;
                    }
					
                    char* mime;
                    if (!strcasecmp(szType, "flv")) {
                        pss->media_type = media_flv;
                        strErrInfo = StartPlayLive(pss, szCode, nChannel);
                        if(!strErrInfo.empty()){
                            break;
                        }
						mime = "video/x-flv";
                    } else if(!strcasecmp(szType, "h264")) {
                        pss->media_type = media_h264;
                        strErrInfo = StartPlayLive(pss, szCode, nChannel);
                        if(!strErrInfo.empty()){
                            break;
                        }
						mime = "video/h264";
                    } else if(!strcasecmp(szType, "mp4")) {
                        pss->media_type = media_mp4;
                        strErrInfo = StartPlayLive(pss, szCode, nChannel);
                        if(!strErrInfo.empty()){
                            break;
                        }
						mime = "video/mp4";
                    } else if(!strcasecmp(szType, "hls")) {
                        char szRealCode[30]={0}, szRealType[30]={0};
                        ret = sscanf(szCode, "%[0-9].[0-9a-z]", szRealCode, szRealType);
                        if(ret < 0) {
                            Log::debug("error hls path %s, %d", szCode, ret);
                            strErrInfo = "error request path";
                            break;
                        }
                        if(!strcasecmp(szRealType, "m3u8")){
                            pss->media_type = media_m3u8;
                            strErrInfo = StartPlayHls(pss, szRealCode);
                            if(!strErrInfo.empty()){
                                break;
                            }
                            mime = "application/x-mpegURL";
                        } else if(!strcasecmp(szRealType, "ts")) {
                            pss->media_type = media_ts;
                            strErrInfo = StartPlayHls(pss, szRealCode);
                            if(!strErrInfo.empty()){
                                break;
                            }
                            mime = "video/MP2T";
                        } else {
                            strErrInfo = "error request path";
                            break;
                        }
                    } else {
                        strErrInfo = "error request path";
                        break;
                    }
                    lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Access-Control-Allow-Origin", (const uint8_t *)"*", 1, &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Content-Type", (const uint8_t *)mime, strlen(mime), &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Cache-Control", (const uint8_t *)"no-cache", 8, &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Expires", (const uint8_t *)"-1", 2, &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Pragma", (const uint8_t *)"no-cache", 8, &p, end);
                    if (lws_finalize_write_http_header(wsi, start, &p, end))
                        return 1;

                    if(!strcasecmp(szType, "hls"))
                        lws_callback_on_writable(wsi);
                    return 0;
                }while(0);

                lws_snprintf(pss->strErrInfo, sizeof(pss->strErrInfo), "%s", strErrInfo.c_str());

                if (lws_add_http_common_headers(wsi, HTTP_STATUS_FORBIDDEN, "text/html",
                    LWS_ILLEGAL_HTTP_CONTENT_LEN, &p, end))
                    return 1;
                if (lws_finalize_write_http_header(wsi, start, &p, end))
                    return 1;

                lws_callback_on_writable(wsi);
                return 0;
            }
        case LWS_CALLBACK_HTTP_WRITEABLE: 
            {
                if (!pss || pss->culled)
                    break;

                if (strlen(pss->strErrInfo) == 0) {

                    if(pss->media_type == media_flv){
                        SendLiveFlv(pss);
                    } else if(pss->media_type == media_h264){
                        SendLiveH264(pss);
                    } else if(pss->media_type == media_mp4){
                        SendLiveMp4(pss);
                    } else if(pss->media_type == media_m3u8){
                        if(SendLiveM3u8(pss)){
                            if (lws_http_transaction_completed(wsi))
                                return -1;
                        }
                    } else if(pss->media_type == media_ts){
                        if(SendLiveTs(pss)){
                            if (lws_http_transaction_completed(wsi))
                                return -1;
                        } else {
                            if (lws_http_transaction_completed(wsi))
                                return -1;
                        }
                    }
                    //lws_callback_on_writable(wsi);
                    return 0;
                } else {
                    int len = strlen(pss->strErrInfo);
                    int wlen = lws_write(wsi, (uint8_t *)pss->strErrInfo, len, LWS_WRITE_HTTP_FINAL);
                    if (wlen != len)
                        return 1;
                    if (lws_http_transaction_completed(wsi))
                        return -1;
                    return 0;
                }

                return 0;
            }
        case LWS_CALLBACK_CLOSED_HTTP:
            {
                if (!pss || !pss->m_pWorker)
                    break;
				if(pss->media_type == media_m3u8) {
					//CHlsWorker* pWorker = (CHlsWorker*)pss->m_pWorker;
					//pWorker->DelConnect(pss);
				} else if(pss->media_type == media_ts) {
				} else {
					pss->m_pWorker->DelConnect(pss);
				}
            }
        default:
            break;
        }

        return lws_callback_http_dummy(wsi, reason, user, in, len);
    }

    int callback_live_ws(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
    {
        pss_http_ws_live *pss = (pss_http_ws_live*)user;

        switch (reason) 
        {
        case LWS_CALLBACK_PROTOCOL_INIT:
            Log::debug("live ws protocol init");
            break;
        case LWS_CALLBACK_ESTABLISHED:
            {
                /* add ourselves to the list of live pss held in the vhd */
                lws_hdr_copy(wsi, pss->path, sizeof(pss->path), WSI_TOKEN_GET_URI);
                Log::debug("live ws protocol establised: %s", pss->path);

                pss->wsi = wsi;
                pss->pss_next = nullptr;
                pss->isWs = true;
                pss->m_bSendHead = false;
                pss->m_pWorker = nullptr;

                string strErrInfo;
                string strPath = pss->path;
				// ws://IP:port/live/type/channel/code
                // type: flv、mp4、h264、hls
                // channel: 0:原始码流 1:小码流
                // e.g ws://localhost:80/live/flv/0/123456789
                // 这里path只有 /live/type/stream/code
                do{
                    char szType[10]={0}, szCode[30]={0};
                    int nChannel = 0;
                    int ret = sscanf(pss->path, "/live/%[a-z0-9]/%d/%[0-9a-z]", szType, &nChannel, szCode);
                    if(ret < 0) {
                        Log::error("%s error sscanf %d", pss->path, ret);
                        strErrInfo = "error request path";
                        break;
                    }

                    if (!strcasecmp(szType, "flv")) {
                        pss->media_type = media_flv;
                        strErrInfo = StartPlayLive(pss, szCode, nChannel);
                        if(!strErrInfo.empty()){
                            break;
                        }
                    } else if(!strcasecmp(szType, "h264")) {
                        pss->media_type = media_h264;
                        strErrInfo = StartPlayLive(pss, szCode, nChannel);
                        if(!strErrInfo.empty()){
                            break;
                        }
                    } else if(!strcasecmp(szType, "mp4")) {
                        pss->media_type = media_mp4;
                        strErrInfo = StartPlayLive(pss, szCode, nChannel);
                        if(!strErrInfo.empty()){
                            break;
                        }
                    } else {
                        strErrInfo = "error request path";
                        break;
                    }

				// 这个操作耗时5s	
                lws_get_peer_addresses(wsi, lws_get_socket_fd(wsi),
                    pss->clientName, sizeof(pss->clientName),
                    pss->clientIP, sizeof(pss->clientIP));
                    return 0;
                }while(0);

                //播放失败，断开连接
                lws_snprintf(pss->strErrInfo, sizeof(pss->strErrInfo), "%s", (const char *)strErrInfo.c_str());
                int len = strlen(pss->strErrInfo);
                lws_close_reason(wsi, LWS_CLOSE_STATUS_NORMAL,(uint8_t *)pss->strErrInfo, len);
                return 0;
            }
            break;
        case LWS_CALLBACK_RECEIVE: 
            {
                if (!pss)
                    break;
                string strRecv((char*)in,len);
                Log::debug("live ws protocol recv len:%d", len);
                Log::debug(strRecv.c_str());
            }
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE: 
            {
                if (!pss || pss->culled)
                    break;

                //Log::debug("live ws protocol writeable %s", pss->path);
                int len = 0;
                int wlen = 0;
                if(pss->media_type == media_flv){
                    SendLiveFlv(pss);
                } else if(pss->media_type == media_h264) {
                    SendLiveH264(pss);
                } else if(pss->media_type == media_mp4){
                    SendLiveMp4(pss);
                }

                return 0;
            }
        case LWS_CALLBACK_CLOSED:
            {
                if (!pss || !pss->m_pWorker)
                    break;
                Log::debug("live ws protocol cloes %s", pss->path);
                pss->m_pWorker->DelConnect(pss);
            }
        default:
            break;
        }

        return 0;
    }
};