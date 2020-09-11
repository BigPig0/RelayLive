#include "uvlogconf.h"
#include "pugixml.hpp"
#include "cJSON.h"
#include "utilc_api.h"
#include <string>

namespace uvLogPlus {
    Level level_parse(const char *lv) {
        Level level = Level::OFF;
        if(!strcasecmp(lv, "Trace"))
            level = Level::Trace;
        else if(!strcasecmp(lv, "Debug"))
            level = Level::Debug;
        else if(!strcasecmp(lv, "Info"))
            level = Level::Info;
        else if(!strcasecmp(lv, "Warn"))
            level = Level::Warn;
        else if(!strcasecmp(lv, "Error"))
            level = Level::Error;
        else if(!strcasecmp(lv, "Fatal"))
            level = Level::Fatal;
        else if(!strcasecmp(lv, "All"))
            level = Level::All;
        return level;
    }

    static Configuration* ConfigParseJsonBuff(const char* conf_buff) {
        Configuration* ret = NULL;
        cJSON* root = cJSON_Parse(conf_buff);
        if(NULL == root)
            return NULL;
        cJSON* conf = cJSON_GetObjectItemCaseSensitive(root, "configuration");
        if(NULL == conf)
            return NULL;
        cJSON* apds = cJSON_GetObjectItemCaseSensitive(conf, "appenders");
        cJSON* logs = cJSON_GetObjectItemCaseSensitive(conf, "loggers");
        if(NULL == apds || NULL == logs)
            return NULL;

        ret = new Configuration;

        // 解析所有appender
        int size = cJSON_GetArraySize(apds);
        for(int i=0; i<size; i++) {
            cJSON* apd = cJSON_GetArrayItem(apds, i);
            if(apd->type == cJSON_Object) {
                cJSON* name = cJSON_GetObjectItemCaseSensitive(apd, "name");
                if(NULL == name || name->type != cJSON_String)
                    continue;
                if(!strcasecmp(apd->string, "console")) {
                    ConsolAppender *appender = new ConsolAppender();
                    appender->name = name->valuestring;
                    appender->type = AppenderType::consol;
                    cJSON* tar = cJSON_GetObjectItemCaseSensitive(apd, "target");
                    if(tar->type == cJSON_String && !strcasecmp(tar->valuestring, "SYSTEM_ERR"))
                        appender->target = ConsolTarget::SYSTEM_ERR;
                    ret->appenders.insert(std::make_pair(appender->name, appender));
                } else if(!strcasecmp(apd->string, "RollingFile")) {
                    RollingFileAppender *appender = new RollingFileAppender();
                    appender->name = name->valuestring;
                    appender->type = AppenderType::rolling_file;
                    cJSON* fn = cJSON_GetObjectItemCaseSensitive(apd, "fileName");
                    if(fn && fn->type == cJSON_String)
                        appender->file_name = fn->valuestring;
                    cJSON* pl = cJSON_GetObjectItemCaseSensitive(apd, "Policies");
                    if(pl && pl->type == cJSON_Object){
                        cJSON* sz = cJSON_GetObjectItemCaseSensitive(pl, "size");
                        if(sz && sz->type == cJSON_String) {
                            std::string num,type,value(sz->valuestring);
                            for(auto c:value) {
                                if(c>='0' && c<='9') {
                                    num += c;
                                } else if(c == 'K' || c == 'k') {
                                    appender->policies.size_policy.size = stoi(num) * 1024;
                                    break;
                                } else if(c == 'M' || c == 'm') {
                                    appender->policies.size_policy.size = stoi(num) * 1024 * 1024;
                                    break;
                                } else if(c == 'G' || c == 'g') {
                                    appender->policies.size_policy.size = stoi(num) * 1024 * 1024 * 1024;
                                    break;
                                }
                            }
                        } else if(sz && sz->type == cJSON_Number){
                            appender->policies.size_policy.size = sz->valueint;
                        }
                        cJSON* mx = cJSON_GetObjectItemCaseSensitive(pl, "max");
                        if(mx && mx->type == cJSON_Number)
                            appender->max = mx->valueint;
                    }
                    ret->appenders.insert(std::make_pair(appender->name, appender));
                } else if(!strcasecmp(apd->string, "file")) {
                    FileAppender *appender = new FileAppender();
                    appender->name = name->valuestring;
                    appender->type = AppenderType::file;
                    cJSON* fn = cJSON_GetObjectItemCaseSensitive(apd, "fileName");
                    if(fn && fn->type == cJSON_String)
                        appender->file_name = fn->valuestring;
                    cJSON* ad = cJSON_GetObjectItemCaseSensitive(apd, "append");
                    if(ad && ((ad->type == cJSON_Number && ad->valueint > 0) || 
                        (ad->type == cJSON_String && (!strcasecmp(ad->valuestring, "yes") || !strcasecmp(ad->valuestring, "true")))))
                        appender->append = true;
                    ret->appenders.insert(std::make_pair(appender->name, appender));
                } 
            }
        }

        //解析所有logger
        size = cJSON_GetArraySize(logs);
        for(int i=0; i<size; i++) {
            cJSON* log = cJSON_GetArrayItem(logs, i);
            if(log->type == cJSON_Object) {
                Logger *logger = new Logger;
                logger->name = log->string;
                logger->additivity = false;
                logger->level = Level::OFF;
                int attr_size = cJSON_GetArraySize(log);
                for(int j=0; j<attr_size; j++) {
                    cJSON* attr = cJSON_GetArrayItem(log, j);
                    if(attr->type == cJSON_String && !strcasecmp(attr->string, "level")) {
                        logger->level = level_parse(attr->valuestring);
                    } else if(attr->type == cJSON_Object && !strcasecmp(attr->string, "appender-ref")) {
                        cJSON* ref = cJSON_GetObjectItemCaseSensitive(attr, "ref");
                        if(ref && ref->type == cJSON_String)
                            logger->appender_ref.push_back(ref->valuestring);
                    }
                }
                if(!strcasecmp(log->string, "root")) {
                    ret->root = logger;
                } else {
                    if(ret->loggers.count(logger->name) == 0)
                        ret->loggers.insert(make_pair(logger->name, logger));
                    else
                        delete logger;
                }
            }
        }

        return ret;
    }

    static Configuration* ConfigParseXmlBuff(const char* conf_buff) {
        Configuration* ret = NULL;
        pugi::xml_document doc;
        do
        {
            if (!doc.load(conf_buff)) {
                break;
            }

            // 根节点
            pugi::xml_node root = doc.child("configuration");
            if (!root)
                break;
            ret = new Configuration;

            //appenders节点
            pugi::xml_node node_appenders = root.child("appenders");
            if(node_appenders) {
                for (pugi::xml_node node = node_appenders.first_child(); node; node = node.next_sibling()) {
                    std::string nodeName  = node.name();
                    if(!strcasecmp(nodeName.c_str(), "console")) {
                        ConsolAppender *appender = new ConsolAppender();
                        for(pugi::xml_node::attribute_iterator iter = node.attributes_begin();
                            iter != node.attributes_end(); ++iter){
                                if(!strcasecmp(iter->name(),"name"))
                                    appender->name = iter->value();
                                else if(!strcasecmp(iter->name(),"target")) {
                                    if(!strncasecmp(iter->value(),"SYSTEM_ERR", 10))
                                        appender->target = ConsolTarget::SYSTEM_ERR;
                                }
                        }
                        ret->appenders.insert(make_pair(appender->name, appender));
                    } else if(!strcasecmp(nodeName.c_str(), "File")) {
                        FileAppender *appender = new FileAppender();
                        for(pugi::xml_node::attribute_iterator iter = node.attributes_begin();
                            iter != node.attributes_end(); ++iter){
                                if(!strcasecmp(iter->name(),"name"))
                                    appender->name = iter->value();
                                else if(!strcasecmp(iter->name(),"fileName"))
                                    appender->file_name = iter->value();
                                else if(!strcasecmp(iter->name(),"append"))
                                    if(!strcasecmp(iter->value(),"true") || !strcasecmp(iter->value(),"yes") || !strcasecmp(iter->value(),"1"))
                                        appender->append = true;
                        }
                        ret->appenders.insert(make_pair(appender->name, appender));
                    } else if(!strcasecmp(nodeName.c_str(), "RollingFile")) {
                        RollingFileAppender *appender = new RollingFileAppender();
                        for(pugi::xml_node::attribute_iterator iter = node.attributes_begin();
                            iter != node.attributes_end(); ++iter){
                                if(!strcasecmp(iter->name(),"name"))
                                    appender->name = iter->value();
                                else if(!strcasecmp(iter->name(),"fileName"))
                                    appender->file_name = iter->value();
                                else if(!strcasecmp(iter->name(),"filePattern"))
                                    appender->filePattern = iter->value();
                        }
                        ret->appenders.insert(make_pair(appender->name, appender));
                    }
                }
            }

            //loggers节点
            pugi::xml_node node_loggers = root.child("loggers");

         }while(0);
        return ret;
    }

    Configuration* ConfigParse(const char *buff) {
        Configuration* conf = ConfigParseJsonBuff(buff);
        if(NULL == conf)
            conf = ConfigParseXmlBuff(buff);
        return conf;
    }

    Configuration* ConfigParse(uv_file file) {
        char *base = (char*)calloc(1024,1024);
        uv_buf_t buffer = uv_buf_init(base, 1024*1024);
        uv_fs_t read_req;
        int ret = uv_fs_read(NULL, &read_req, file, &buffer, 1, -1, NULL);
        if(ret < 0) {
            printf("uv fs read failed:%s\n",uv_strerror(ret));
            return NULL;
        }

        return ConfigParse(base);
    }

    Configuration* ConfigParse(std::string path) {
        uv_fs_t open_req;
        int ret = uv_fs_open(NULL, &open_req, path.c_str(), O_RDONLY, 0, NULL);
        if(ret < 0) {
            printf("uv fs open %s failed:%s\n", path.c_str(), uv_strerror(ret));
            return NULL;
        }
        Configuration* conf = ConfigParse(open_req.result);
        uv_fs_close(NULL, &open_req, open_req.result, NULL);

        return conf;
    }
}