#include "stdafx.h"
#include "RtspLiveServer.h"
#include "RtspWorker.h"
#include "cstl_easy.h"

namespace RtspServer
{
    void make_option_answer(pss_rtsp_client *pss){
        Log::debug("make_option_answer");
        CRtspSocket *client = pss->rtspClient;
        rtsp_ruquest_t *req = client->m_Request;
        rtsp_response *res = client->m_Response;

        CRtspWorker* pWorker = GetRtspWorker(pss->code);
        if(!pWorker) {
            pWorker = CreatRtspWorker(pss->code);
        }
        if(!pWorker){
            Log::error("CreatRtspWorker failed%s", pss->code);
            res->code = Code_404_NotFound;
        } else {
            res->code = Code_200_OK;
            pWorker->AddConnect(pss);
            pss->m_pWorker = pWorker;
        }

        res->headers.insert(make_pair("Public","OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE"));
    }

    void make_describe_answer(pss_rtsp_client *pss)
    {
        Log::debug("make_describe_answer");
        CRtspWorker* pWorker  = pss->m_pWorker;
        CRtspSocket *client   = pss->rtspClient;
        CRtspSession *session = client->m_pSession;
        rtsp_ruquest_t *req   = client->m_Request;
        rtsp_response *res    = client->m_Response;

        res->code = Code_200_OK;

		stringstream ss;
		ss << "v=0\r\n"
			<< "o=" << client->m_strDevCode << " 0 0 IN IP4 " << client->m_strRtpIP << "\r\n"
			<< "s=" << client->m_strDevCode << "\r\n"
			<< "c=IN IP4 " << client->m_strLocalIP << "\r\n"
			<< "t=0 0\r\n"
			<< "a=sdplang:en\r\n"
			<< "a=range:npt=0-\r\n"
			<< "a=control:*\r\n"
			<< "m=video 0 RTP/AVP 96\r\n"
			<< "a=rtpmap:96 MP2P/90000\r\n"
			<< "a=recvonly\r\n"
            << "a=control:trackID=1\r\n"
			;

		res->body = ss.str();
        res->headers.insert(make_pair("Content-Type","application/sdp"));
        res->headers.insert(make_pair("Content-Length",StringHandle::toStr<size_t>(res->body.size())));

        session->NewSubSession("trackID=1");
    }

    void make_setup_answer(pss_rtsp_client *pss)
    {
        Log::debug("make_setup_answer");
        CRtspSocket *client = pss->rtspClient;
        rtsp_ruquest_t *req = client->m_Request;
        rtsp_response *res  = client->m_Response;

        char code[50]={0};
        char control[20]={0};
        int nChannel = 0;
        sscanf(pss->path, "rtsp://%[^/]/live/%[^/]/%d/%s", code, &nChannel, control);

        CRtspSession *session = client->m_pSession;
        CRtspSubSession *subSession = client->m_pSession->GetSubSession(control);

        string_t* str_trans = (string_t*)hash_map_find_easy_str(req->headers, "Transport");
        if(!str_trans) {
            res->code = Code_400_BadRequest;
            return;
        } else {
            bool port = false;
            string trans = string_c_str(str_trans);
            res->headers.insert(make_pair("Transport", trans));
            vector<string> vec_trans = StringHandle::StringSplit(trans, ';');
            for (auto& it:vec_trans){
                if(it == "RTP/AVP"){
                    subSession->m_nUseTcp = 0;
                } else if(it == "RTP/AVP/TCP"){
                    subSession->m_nUseTcp = 1;
                } else {
                    if(it.size() > 12 && !it.compare(0,12,"client_port=",0,12)) {
                        sscanf(it.c_str(), "client_port=%d-%d", &subSession->m_nRtpPort, &subSession->m_nRtcpPort);
                    } else if(it.size() > 12 && !it.compare(0, 12, "interleaved=", 0, 12)) {
                        sscanf(it.c_str(), "interleaved=%d-%d", &subSession->m_nRtpPort, &subSession->m_nRtcpPort);
                    }
                }
            }
        }
        res->code = Code_200_OK;
        return;
    }

    void make_play_answer(pss_rtsp_client *pss)
    {
        Log::debug("make_play_answer");
        CRtspSocket *client = pss->rtspClient;
        rtsp_ruquest_t *req = client->m_Request;
        rtsp_response *res  = client->m_Response;
        string_t* str_session = (string_t*)hash_map_find_easy_str(req->headers, "Session");
       
        if (!str_session) {
            res->code = Code_400_BadRequest;
            return;
        }

        string session = string_c_str(str_session);
        res->headers.insert(make_pair("Session",session));
        res->code = Code_200_OK;
        pss->playing = true;
    }

    void make_pause_answer(pss_rtsp_client *pss)
    {
        Log::debug("make_pause_answer");
        CRtspSocket *client = pss->rtspClient;
        rtsp_ruquest_t *req = client->m_Request;
        rtsp_response *res  = client->m_Response;
		res->code = Code_200_OK;
    }

    void make_teardown_answer(pss_rtsp_client *pss)
    {
        Log::debug("make_teardown_answer");
        CRtspSocket *client = pss->rtspClient;
        rtsp_ruquest_t *req = client->m_Request;
        rtsp_response *res  = client->m_Response;

        string_t* str_session = (string_t*)hash_map_find_easy_str(req->headers, "Session");
        if (!str_session) {
            res->code = Code_400_BadRequest;
        }
        string session = string_c_str(str_session);

        res->headers.insert(make_pair("Session",session));
        res->code = Code_200_OK;
        pss->playing = false;
    }

    int callback_live_rtsp(CRtspSocket *client, RTSP_REASON reason, void *user)
    {
        pss_rtsp_client *pss = (pss_rtsp_client*)user;

        switch (reason) {
        case RTSP_REASON_OPTIONS:
            {
				memcpy(pss->path, client->m_Request->uri, strlen(client->m_Request->uri));
                Log::debug("new options request: %s", pss->path);
				
                sscanf(pss->path, "rtsp://%[^/]/live/%[^/]/%d", pss->code, &pss->channel);
                pss->rtspClient = client;
                pss->pss_next = nullptr;
                pss->m_pWorker = nullptr;
                pss->playing = false;

               
                make_option_answer(pss);
            }
            break;
        case RTSP_REASON_DESCRIBE:
            {
                 if (!pss )
                    break;

                make_describe_answer(pss);
            }
            break;
        case RTSP_SETUP:
            {
                if (!pss )
                    break;
                
                make_setup_answer(pss);
            }
            break;
        case RTSP_REASON_PLAY:
            {
                if (!pss)
                    break;
                make_play_answer(pss);
            }
            break;
        case RTSP_REASON_PAUSE:
            {
                if (!pss || !pss->m_pWorker)
                    break;
                make_pause_answer(pss);
            }
            break;
        case RTSP_REASON_TEARDOWN:
            {
                if (!pss)
                    break;
                make_teardown_answer(pss);
            }
            break;
        case RTSP_REASON_CLOSE:
            {
                if (!pss || !pss->m_pWorker)
                    break;
                pss->m_pWorker->DelConnect(pss);
            }
            break;
        case RTSP_REASON_WRITE:
            break;
        case RTSP_REASON_RTP_WRITE:
            {
                if (!pss || !pss->m_pWorker)
                    break;

                AV_BUFF pkg = pss->m_pWorker->GetVideo(&pss->tail);
                char* buff = (char *)malloc(pkg.nLen);
                memcpy(buff, pkg.pData, pkg.nLen);
                rtp_write(client, buff, pkg.nLen);
                pss->m_pWorker->NextWork(pss);
            }
            break;
        case RTSP_REASON_RTCP_WRITE:
            break;
        default:
            break;
        }
        return 0;
    }

}