#include "stdafx.h"
#include "RtspLiveServer.h"
#include "RtspWorker.h"

namespace RtspServer
{
    rtsp_response make_option_answer(rtsp_ruquest req)
    {
        Log::debug("make_option_answer");
        rtsp_response res;
        res.code = Code_200_OK;
        res.headers.insert(make_pair("Public","OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE"));
        return res;
    }

    rtsp_response make_describe_answer(CRtspWorker* pWorker, CClient *client)
    {
        Log::debug("make_describe_answer");
        rtsp_ruquest req = *client->m_Request;
        rtsp_response res;
        res.code = Code_200_OK;
        res.body = pWorker->GetSDP();
        res.headers.insert(make_pair("Content-Type","application/sdp"));
        res.headers.insert(make_pair("Content-Length",StringHandle::toStr<size_t>(res.body.size())));
        return res;
    }

    rtsp_response make_setup_answer(CClient *client)
    {
        Log::debug("make_setup_answer");
        rtsp_ruquest &req = *client->m_Request;
        rtsp_response res;
        auto tra_it = req.headers.find("Transport");
        if(tra_it == req.headers.end()) {
            res.code = Code_400_BadRequest;
            return res;
        } else {
            bool port = false;
            string trans = tra_it->second;
            res.headers.insert(make_pair("Transport", trans));
            vector<string> vec_trans = StringHandle::StringSplit(trans, ';');
            for (auto& it:vec_trans){
                if (it.size() > 12) {
                    if(0 == it.compare(0,12,"client_port=",0,12)) {
                        size_t pos = it.find('-',12);
                        if (pos == string::npos) {
                            res.code = Code_461_UnsupportedTransport;
                            return res;
                        }
                        string rtpport = it.substr(12,pos-12);
                        string rtcpport = it.substr(pos+1, it.size()-pos-1);
                        //m_nRtpPort = stoi(rtpport);
                        try{
                            req.rtp_port = stoi(rtpport);
                            req.rtcp_port= stoi(rtcpport);
                            client->SetRemotePort(req.rtp_port, req.rtcp_port);
                            port = true;
                            break;
                        } catch (...) {
                            res.code = Code_461_UnsupportedTransport;
                            return res;
                        }
                    }
                }
            }
        }
        string session = StringHandle::toStr<uint64_t>(CRtspServer::m_nSession++);
        res.headers.insert(make_pair("Session",session));
        res.code = Code_200_OK;
        return res;
    }

    rtsp_response make_play_answer(rtsp_ruquest req)
    {
        Log::debug("make_play_answer");
        rtsp_response res;
        auto sess_it = req.headers.find("Session");
        if (sess_it == req.headers.end()) {
            res.code = Code_400_BadRequest;
            return res;
        }

        string session = sess_it->second;

        //m_strDevCode = "36030100061320000026";
        //string rtpIP="172.31.7.88";

        res.headers.insert(make_pair("Session",session));
        res.code = Code_200_OK;
        return res;
    }

    rtsp_response make_pause_answer(rtsp_ruquest req)
    {
        Log::debug("make_pause_answer");
        rtsp_response res;
        return res;
    }

    rtsp_response make_teardown_answer(rtsp_ruquest req)
    {
        Log::debug("make_teardown_answer");
        rtsp_response res;
        auto sess_it = req.headers.find("Session");
        if (sess_it == req.headers.end()) {
            res.code = Code_400_BadRequest;
            return res;
        }
        string session = sess_it->second;

        res.headers.insert(make_pair("Session",session));
        res.code = Code_200_OK;
        return res;
    }

    int callback_live_rtsp(CClient *client, rtsp_method reason, void *user, void *in, size_t len)
    {
        pss_rtsp_client *pss = (pss_rtsp_client*)user;

        switch (reason) {
        case RTSP_OPTIONS:
            {
                pss->path = string((char*)in, len); //live/123456789
                Log::debug("new request: %s", pss->path.c_str());

                pss->rtspClient = client;
                pss->pss_next = nullptr;
                pss->m_pWorker = nullptr;
                pss->playing = false;

                if(!strncasecmp((char*)in, "live/", 5)){
                    pss->code = string((char*)in+5, len-5);
                    CRtspWorker* pWorker = GetRtspWorker(pss->code);
                    if(!pWorker) {
                        pWorker = CreatRtspWorker(pss->code);
                    }
                    if(!pWorker){
                        Log::error("CreatRtspWorker failed%s", pss->code.c_str());
                    } else {
                        pWorker->AddConnect(pss);
                        pss->m_pWorker = pWorker;
                        *client->m_Response = make_option_answer(*client->m_Request);
                        return 0;
                    }
                }
                client->m_Response->code = Code_400_BadRequest;
                return 1;
            }
            break;
        case RTSP_DESCRIBE:
            {
                 if (!pss || !pss->m_pWorker)
                    break;
                string strPath = string((char*)in, len);
                if(strPath != pss->path){
                    client->m_Response->code = Code_400_BadRequest;
                    return 1;
                }

                *client->m_Response = make_describe_answer(pss->m_pWorker, client);
            }
            break;
        case RTSP_SETUP:
            {
                if (!pss || !pss->m_pWorker)
                    break;
                string strPath = string((char*)in, len);
                if(strPath != pss->path){
                    client->m_Response->code = Code_400_BadRequest;
                    return 1;
                }
                *client->m_Response = make_setup_answer(client);
            }
            break;
        case RTSP_PLAY:
            {
                if (!pss || !pss->m_pWorker)
                    break;
                string strPath = string((char*)in, len);
                if(strPath != pss->path){
                    client->m_Response->code = Code_400_BadRequest;
                    return 1;
                }
                *client->m_Response = make_play_answer(*client->m_Request);
                if(client->m_Response->code == Code_200_OK){
                    pss->playing = true;
                }
            }
            break;
        case RTSP_PAUSE:
            {
                if (!pss || !pss->m_pWorker)
                    break;
                *client->m_Response = make_pause_answer(*client->m_Request);
            }
            break;
        case RTSP_TEARDOWN:
            {
                if (!pss || !pss->m_pWorker)
                    break;
                string strPath = string((char*)in, len);
                if(strPath != pss->path){
                    client->m_Response->code = Code_400_BadRequest;
                    return 1;
                }
                *client->m_Response = make_teardown_answer(*client->m_Request);
                if(client->m_Response->code == Code_200_OK){
                    pss->playing = false;
                }
            }
            break;
        case RTSP_CLOSE:
            {
                if (!pss || !pss->m_pWorker)
                    break;
                pss->m_pWorker->DelConnect(pss);
            }
            break;
        case RTSP_WRITE:
            break;
        case RTP_WRITE:
            {
                if (!pss || !pss->m_pWorker)
                    break;

                LIVE_BUFF pkg = pss->m_pWorker->GetVideo(&pss->tail);
                char* buff = (char *)malloc(pkg.nLen);
                memcpy(buff, pkg.pBuff, pkg.nLen);
                rtp_write(client, buff, pkg.nLen);
                pss->m_pWorker->NextWork(pss);
            }
            break;
        case RTCP_WRITE:
            break;
        default:
            break;
        }
        return 0;
    }

}