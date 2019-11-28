#include "util.h"
#include "libwebsockets.h"
#include "web.h"
#include <string>
#include <set>

namespace Server
{
#define G_BYTES (1024 * 1024 * 1024) // 1GB
#define M_BYTES (1024 * 1024)		 // 1MB
#define K_BYTES 1024				 // 1KB

    struct asyncRequest {
        static std::set<pss_device*>  set_all;      // 所有请求的集合
        static CriticalSection        cs_all;

        std::set<pss_device*>  set_req;     // 指定请求的集合
        CriticalSection        cs_req;
        bool                   is_quering;
        string                 cmd;

        struct asyncRequest(string str) 
            : is_quering(false) 
            , cmd(str)
        {}

        static void add_all(pss_device *pss) {
            MutexLock lock(&cs_all);
            set_all.insert(pss);
        }
        static void remove_all(pss_device *pss) {
            MutexLock lock(&cs_all);
            auto it = set_all.find(pss);
            if(it != set_all.end())
                set_all.erase(it);
        }
        void add_req(pss_device *pss) {
            MutexLock lock(&cs_req);
            set_req.insert(pss);
            if(!is_quering){
                is_quering = true;
                if(cmd == "devlist")
                    LiveClient::GetDevList();
                else if(cmd == "clientsinfo")
                    LiveClient::GetClientsInfo();
            }
        }
        int response(string res) {
            MutexLock lock(&cs_req);
            for(auto pss : set_req) {
                //判断请求是否依然存在
                MutexLock lk(&cs_all);
                auto it = set_all.find(pss);
                if(it == set_all.end()){
                    Log::warning("http request connect has removed");
                    continue;
                }

                struct lws *wsi = pss->wsi;
                *pss->response_body = res;

                uint8_t buf[LWS_PRE + 2048], 
                    *start = &buf[LWS_PRE], 
                    *p = start,
                    *end = &buf[sizeof(buf) - LWS_PRE - 1];

                lws_add_http_common_headers(wsi, HTTP_STATUS_OK, "text/html", pss->response_body->size(), &p, end);
                lws_add_http_header_by_name(wsi, (const uint8_t *)"Access-Control-Allow-Origin", (const uint8_t *)"*", 1, &p, end);
                if (lws_finalize_write_http_header(wsi, start, &p, end))
                    return 1;

                lws_callback_on_writable(wsi);
            }
            set_req.clear();
            is_quering = false;

            return 0;
        }
    };
    std::set<pss_device*>  asyncRequest::set_all;
    CriticalSection        asyncRequest::cs_all;
    asyncRequest devlist_request("devlist");
    asyncRequest clients_request("clientsinfo");

    static std::string mount_web_origin("./home");  //站点本地位置

    static uint32_t make_dir_info(string urlStr, string path, string &strHtml) {
        std::string strFiles(""); // 普通文件写在这个字符串中.
        char buffer[MAX_PATH + 100] = {0};
        char sizeBuf[MAX_PATH + 100] = {0};
        // 1. 输出HTML头,并指定UTF-8编码格式
        strHtml = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/></head>";
        strHtml.append("<body>");
        // 2. 输出路径
        //(1). 输出第一项 根目录
        strHtml.append("<A href=\"/\">/</A>");
        //(2). 其它目录
        uv_fs_t req;
        uv_dirent_t dent;
        int sr = uv_fs_scandir(NULL, &req, path.c_str(), 0, NULL);
        if(sr < 0) {
            strHtml = "error path is not dir";
            return HTTP_STATUS_FORBIDDEN;
        }
        std::string::size_type st = 1;
        std::string::size_type stNext = 1;
        while( (stNext = urlStr.find('/', st)) != std::string::npos)
        {
            std::string strDirName =  urlStr.substr(st, stNext - st + 1);
            std::string strSubUrl = urlStr.substr(0, stNext + 1);

            //strHtml.append("&nbsp;|&nbsp;");

            strHtml.append("<A href=\"");
            strHtml.append(strSubUrl);
            strHtml.append("\">");
            strHtml.append(strDirName);
            strHtml.append("</A>");

            // 下一个目录
            st = stNext + 1;
        }
        strHtml.append("<br /><hr />");
        // 3. 列出当前目录下的所有文件
        while (uv_fs_scandir_next(&req, &dent) != UV_EOF) {
            Log::debug("find dir:%s", dent.name);
            // 跳过 . 文件
            if( _stricmp(dent.name, ".") == 0 || 0 == _stricmp(dent.name, "..") )
                continue;

            if (dent.type & UV_DIRENT_DIR)
            {
                // 如果是子目录,直接写入
                string sub_path = path;
                sub_path.append(dent.name);
                uv_fs_t stareq;
                uv_fs_stat(NULL, &stareq, sub_path.c_str(), NULL);
                uv_stat_t* stat = uv_fs_get_statbuf(&stareq);
                time_t t = (time_t)stat->st_mtim.tv_sec;
                CTimeFormat::printTime(&t, "%Y-%m-%d %H:%M:%S", buffer);
                strHtml.append(buffer);
                uv_fs_req_cleanup(&stareq);

                // 目录名需要转换为UTF8编码
                sprintf_s(buffer, MAX_PATH + 100, "%s/", dent.name);
                std::string fileurl = urlStr;
                std::string filename = buffer;

                strHtml.append("&nbsp;&nbsp;");
                strHtml.append("<A href=\"");
                strHtml.append(fileurl.c_str());
                strHtml.append(filename.c_str());
                strHtml.append("\">");
                strHtml.append(filename.c_str());
                strHtml.append("</A>");

                // 写入目录标志
                strHtml.append("&nbsp;&nbsp;[DIR]");

                // 换行
                strHtml.append("<br />");
            } else if (dent.type & UV_DIRENT_FILE) {
                // 普通文件,写入到一个缓冲的字符串string变量内,循环外再合并.这样,所有的目录都在前面,文件在后面
                string sub_path = path;
                sub_path.append(dent.name);
                uv_fs_t stareq;
                uv_fs_stat(NULL, &stareq, sub_path.c_str(), NULL);
                uv_stat_t* stat = uv_fs_get_statbuf(&stareq);
                time_t t = (time_t)stat->st_mtim.tv_sec;
                CTimeFormat::printTime(&t, "%Y-%m-%d %H:%M:%S", buffer);
                strFiles += EncodeConvert::AtoUTF8(buffer);

                // 文件名转换为UTF8编码再写入
                std::string filename = dent.name;
                std::string fileurl = urlStr;

                strFiles += "&nbsp;&nbsp;";
                strFiles += "<A href=\"";
                strFiles += fileurl;
                strFiles += filename;
                strFiles += "\">";
                strFiles += filename;
                strFiles += "</A>";

                // 文件大小
                // 注: 由于Windows下 wsprintf 不支持 %f 参数,所以只好用 sprintf 了
                double filesize = 0;
                if( stat->st_size >= G_BYTES)
                {
                    filesize = (stat->st_size * 1.0) / G_BYTES;
                    sprintf_s(sizeBuf, MAX_PATH + 100, "%.2f&nbsp;GB", filesize);
                }
                else if( stat->st_size >= M_BYTES ) // MB
                {
                    filesize = (stat->st_size * 1.0) / M_BYTES;
                    sprintf_s(sizeBuf, MAX_PATH + 100, "%.2f&nbsp;MB", filesize);
                }
                else if( stat->st_size >= K_BYTES ) //KB
                {
                    filesize = (stat->st_size * 1.0) / K_BYTES;
                    sprintf_s(sizeBuf, MAX_PATH + 100, "%.2f&nbsp;KB", filesize);
                }
                else // Bytes
                {
                    sprintf_s(sizeBuf, MAX_PATH + 100, "%lld&nbsp;Bytes", stat->st_size);
                }
                
                strFiles += "&nbsp;&nbsp;";
                strFiles += sizeBuf;

                // 换行
                strFiles += "<br />";
                uv_fs_req_cleanup(&stareq);
            }
        } //while
        uv_fs_req_cleanup(&req);

        // 把文件字符串写入到 Content 中.
        if(strFiles.size() > 0)
        {
            strHtml.append(strFiles.c_str());
        }

        // 4. 输出结束标志.
        strHtml.append("</body></html>");

        return HTTP_STATUS_OK;
    }

    int callback_other_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
    {
        struct pss_other *pss = (struct pss_other *)user;

        switch (reason) {
        case LWS_CALLBACK_HTTP:
            {
                uint8_t buf[LWS_PRE + 2048], 
                    *start = &buf[LWS_PRE], 
                    *p = start,
                    *end = &buf[sizeof(buf) - LWS_PRE - 1];
                lws_snprintf(pss->path, sizeof(pss->path), "%s", (const char *)in);
                Log::debug("new request: %s", pss->path);
                unsigned int code; //返回码
                pss->html = new string();

                //对应的本地路径
                string path;
                static const string home_value = Settings::getValue("HttpServer","RootPath");
                if( !home_value.empty() ) path = home_value;
                else path = mount_web_origin;
                path.append(pss->path);

                static bool bDirVisible = Settings::getValue("HttpServer","DirVisible")=="yes"?true:false;

                //是否存在目录
                code = make_dir_info(pss->path, path, *pss->html);

                if (lws_add_http_common_headers(wsi, code,
                    "text/html",
                    pss->html->size(),
                    &p, end))
                    return 1;
                if (lws_finalize_write_http_header(wsi, start, &p, end))
                    return 1;

                lws_callback_on_writable(wsi);

                return 0;
            }
        case LWS_CALLBACK_HTTP_WRITEABLE:
            {
                if (!pss)
                    break;
                int len = pss->html->size();
                int wlen = lws_write(wsi, (uint8_t *)pss->html->c_str(), len, LWS_WRITE_HTTP_FINAL);
                if (wlen != len)
                    return 1;

                if (lws_http_transaction_completed(wsi))
                    return -1;

                return 0;
            }
        case LWS_CALLBACK_CLOSED_HTTP:
            {
                if (!pss)
                    break;
                SAFE_DELETE(pss->html);
            }

        default:
            break;
        }

        return lws_callback_http_dummy(wsi, reason, user, in, len);
    }

    int callback_device_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
    {
        struct pss_device *pss = (struct pss_device *)user;

        switch (reason) {
        case LWS_CALLBACK_HTTP:
            {
                uint8_t buf[LWS_PRE + 2048], 
                    *start = &buf[LWS_PRE], 
                    *p = start,
                    *end = &buf[sizeof(buf) - LWS_PRE - 1];
                
                asyncRequest::add_all(pss);

				int hlen = lws_hdr_total_length(wsi, WSI_TOKEN_GET_URI);
				if (hlen && hlen < 128) {
					lws_hdr_copy(wsi, pss->path, MAX_PATH, WSI_TOKEN_GET_URI);
					buf[127] = '\0';
				}

				pss->wsi = wsi;
                Log::debug("new http-web request: %s", pss->path);

                pss->response_body = new string;
				if(!strcmp(pss->path, "/device/clients")) {
					clients_request.add_req(pss);
				} else if(!strcmp(pss->path, "/device/devlist")) {
                    devlist_request.add_req(pss);
				} else if(!strcmp(pss->path, "/device/refresh")) {
					LiveClient::QueryDirtionary();

					*pss->response_body = "QueryDirtionary send";
					lws_add_http_common_headers(wsi, HTTP_STATUS_OK, "text/html", pss->response_body->size(), &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Access-Control-Allow-Origin", (const uint8_t *)"*", 1, &p, end);
					if (lws_finalize_write_http_header(wsi, start, &p, end))
						return 1;

					lws_callback_on_writable(wsi);
				} else if(!strncmp(pss->path, "/device/control", 8)) {
					// 设备ID
					char szDev[30]={0};
					sscanf(pss->path, "/device/control/%s", szDev);

					// 参数
					int ud=0, lr=0, io=0;
					char buf[20];
					int n = 0;
					while (lws_hdr_copy_fragment(wsi, buf, sizeof(buf), WSI_TOKEN_HTTP_URI_ARGS, n) > 0) {
						char arg[10]={0};
						int argv = 0;
						sscanf(buf, "%[^=]=%d", arg, &argv);
						if(!strcmp(arg, "ud")) {
							ud = argv;
						} else if(!strcmp(arg, "lr")) {
							lr = argv;
						} else if(!strcmp(arg, "io")) {
							io = argv;
						}
						n++;
					}
					LiveClient::DeviceControl(szDev, io, ud, lr);

					*pss->response_body = "ok";
					lws_add_http_common_headers(wsi, HTTP_STATUS_OK, "text/html", pss->response_body->size(), &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Access-Control-Allow-Origin", (const uint8_t *)"*", 1, &p, end);
					if (lws_finalize_write_http_header(wsi, start, &p, end))
						return 1;

					lws_callback_on_writable(wsi);
				}

                return 0;
            }
        case LWS_CALLBACK_HTTP_WRITEABLE:
            {
                if (!pss)
                    break;

                int len = pss->response_body->size();
                int wlen = lws_write(wsi, (uint8_t *)pss->response_body->c_str(), len, LWS_WRITE_HTTP_FINAL);
                SAFE_DELETE(pss->response_body);
                if (wlen != len)
                    return 1;

                if (lws_http_transaction_completed(wsi))
                    return -1;


                return 0;
            }
        case LWS_CALLBACK_CLOSED_HTTP:
            {
                asyncRequest::remove_all(pss);
                break;
            }
        default:
            break;
        }
        //return 1;
        return lws_callback_http_dummy(wsi, reason, user, in, len);
    }
};
