#include "Task.h"
#include "uvnetplus.h"
#include "easylog.h"
#include "hiksdk.h"
#include <list>
#include <sstream>

using namespace uvNetPlus;
using namespace uvNetPlus::Http;

namespace Task {
    static uv_async_t uvAsync;
    static list<Gps*> gpsList;
    static CriticalSection gpsCs;
    static CNet *cnet;
    static CHttpClient *httpClient;
    static CHttpServer *httpServer;
    static uint64_t uploadNo = 0;
    static string svrip;
    static int svrport;
    static int httpPort;

    static void httpResCB(CHttpRequest *req, CHttpMsg* response) {
        Log::debug(response->content.c_str());
    }

    static void httpReqCB(CHttpRequest *req, void* usr, std::string error) {
        if(!error.empty()) {
            Log::error("http connect failed: %s", error.c_str());
            return;
        }

        Gps *gps = (Gps*)usr;
        std:stringstream ss;
        ss.precision(13);
        ss << "{\"id\":" << util::Guid::getId() << ","
            "\"taskId\":" << gps->taskId << ","
            "\"realTime\":\"" << gps->realTime << "\","
            "\"lat\":" << gps->lat << ","
            "\"lon\":" << gps->lon << ","
            "\"speed\":" << gps->speed << ","
            "\"angle\":" << gps->angle << ","
            "\"uploadNo\":" << uploadNo++ << "}";
        string json = ss.str();
        Log::debug(json.c_str());

        req->path = "/xk/vip/task/updateGps";
        req->method = POST;
        req->OnResponse = httpResCB;
        req->SetHeader("Content-Type", "application/json");
        req->End(json.c_str(), json.size());
    }

    static void on_async_cb(uv_async_t *handle) {
        gpsCs.lock();
        while(!gpsList.empty()) {
            Gps *gps = gpsList.front();
            gpsList.pop_front();
            httpClient->Request(svrip, svrport, gps, httpReqCB);
        }
        gpsCs.unlock();
    }

    static void OnRequestCB(CHttpServer *server, CHttpMsg *request, CHttpResponse *response) {
        Log::debug("rev: %s dont need", request->path.c_str());
        response->statusCode = 404;
        response->End();
    }

//     static void OnPlayReqCB(CHttpServer *server, CHttpMsg *request, CHttpResponse *response) {
//         Log::debug("rev: %s we need", request->path.c_str());
//         char szDevid[50] = {0};
//         int channel = 1;
//         int port = 80;
//         sscanf(request->path.c_str(), "/Play?code=%[^&]&channel=%d&port=%d", szDevid, &channel, &port);
//         PlayParam pa;
//         pa.devId = szDevid;
//         pa.channel = channel;
//         pa.stream = 1; 
//         pa.smsIp = "127.0.0.1";
//         pa.smsPort = port;
//         StopParam res;
//         if(HikSdk::Play(pa, res)){
//             string c = "user="+to_string(res.iUserId)+"&session="+to_string(res.iSession)+"&handle="+to_string(res.iHandle);
//             response->End(c.c_str(), c.size());
//         } else {
//             response->statusCode = 400;
//             string c = "play failed";
//             response->End(c.c_str(), c.size());
//         }
//     }
// 
//     static void OnStopReqCB(CHttpServer *server, CHttpMsg *request, CHttpResponse *response) {
//         Log::debug("rev: %s we need", request->path.c_str());
//         StopParam sp;
//         sscanf(request->path.c_str(),"user=%d&session=%d&&handle=%d", &sp.iUserId, &sp.iSession, &sp.iHandle);
//         HikSdk::Stop(sp);
//         string res = "ok";
//         response->End(res.c_str(), res.size());
//     }

    bool Init(uv_loop_t *loop) {
        svrip = Settings::getValue("Server", "ip");
        svrport = Settings::getValue("Server", "port", 80);
        //httpPort = Settings::getValue("HttpServer", "port", 80);

        uv_async_init(loop, &uvAsync, on_async_cb);
        cnet = CNet::Create(loop);
        httpClient = new CHttpClient(cnet);
//         httpServer = CHttpServer::Create(cnet);
//         httpServer->OnRequest = OnRequestCB;
//         httpServer->On("/Play", OnPlayReqCB);
//         httpServer->On("/Stop", OnStopReqCB);
//         httpServer->Listen("0.0.0.0", httpPort);
        return true;
    }

    void addTask(Gps *gps) {
        gpsCs.lock();
        gpsList.push_back(gps);
        gpsCs.unlock();
        uv_async_send(&uvAsync);
    }
}