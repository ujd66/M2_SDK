#ifndef SC_QACHAIN_API_H
#define SC_QACHAIN_API_H

#include "sc_llm.h"
#include <string>
#include <vector>

namespace SparkChain {




typedef struct Text {
    char * role;
    char * content;
};

typedef struct Header {
    char * app_id;
    char * bot_id;
};

typedef struct Chat {
    double temperature;
    int max_tokens;
    char * chat_id;
};

typedef struct Repo {
    double score;
    int top_k;
};

typedef struct Message {
    std::vector<Text> text;
};

typedef struct Payload {
    Message * message;
};


typedef struct QAParam {
    Header* header;
    struct {
        char* prompt;
        Chat* chat;
        Repo* repo;
    } parameter;
    Payload * payload;
};        


using ChainBaseOutput = LLMBaseOutput;

class SPARKCHAIN_API QAResult : virtual public ChainBaseOutput {
public:
    virtual ~QAResult();

    virtual const char* getRequest() const     = 0;
    virtual const char* getResponse() const    = 0;
};

using QAEvent = LLMEvent;

using QAError = LLMError;

class SPARKCHAIN_API QASyncOutput : virtual public QAResult , virtual public QAError {
public:
    virtual ~QASyncOutput();

};

class SPARKCHAIN_API QACallbacks {
public:
    /**
     *
     * @param key
     * @param request
     * @param response
     * @param usrContext
     */
    virtual void onQAResult(QAResult* result, void* usrContext) = 0;

    /**
     *
     * @param key
     * @param eventMsg
     * @param eventID
     * @param usrContext
     */
    virtual void onQAEvent(QAEvent* event, void* usrContext) = 0;

    /**
     *
     * @param key
     * @param errCode
     * @param errMsg
     * @param usrContext
     */
    virtual void onQAError(QAError* error, void* usrContext) = 0;
};

class SPARKCHAIN_API KnowledgeResult {
public:
    ~KnowledgeResult();

    virtual const char* getSid() const        = 0;
    virtual const char* getResponse() const    = 0;
};

class SPARKCHAIN_API ConfigResult {
public:
    ~ConfigResult();

    virtual const int* getCode() const        = 0;
    virtual const char* getResponse() const    = 0;
};

class SPARKCHAIN_API KnowledgeCallbacks {
public:
    /**
     * @brief 
     * 
     * @param output 
     * @param usrContext 
     */
    virtual void onKnowledgeOutput(KnowledgeResult* output, void* usrContext) = 0;

};

class SPARKCHAIN_API ConfigCallback {
public:
    /**
     * @brief 
     * 
     * @param output 
     * @param usrContext 
     */
    virtual void onResult(const char* output) = 0;

};

/**
 * @brief Agent Api class
 * 
 */
class SPARKCHAIN_API QAChain {

public:

    virtual ~QAChain();

    /**
     * @brief 
     * 
     * @param llm 
     * @param prompt 
     * @return QAChain* 
     */
    static QAChain* create( LLM* llm);
    static QAChain* create();

      /**
     * @brief destroy agent inst
     * 
     */
    static void destroy(QAChain* qaChain);

//同步
    virtual ConfigResult* config(const char* key , const char* value)  = 0 ; // 配置prompt
    virtual ConfigResult* getConfig(const char* key)   = 0; // 获取配置prompt

    virtual QASyncOutput* run(const char*  ids[],const char*  tags[],const char* extend,const char* query ,void* usrTag) = 0;
    virtual QASyncOutput* run(const char* extend,const char* query ,void* usrTag) = 0;


//异步
    virtual void aConfig(const char* key , const char* value,ConfigCallback* configCb)  = 0; // 配置prompt
    virtual void getConfig(const char* key,ConfigCallback* configCb)  = 0; // 获取配置prompt

    virtual void registerQACallbacks(QACallbacks* qaChainCbs) = 0;
    // virtual void registerKnowledgeCallbacks(KnowledgeCallbacks* knowledgeCbs) = 0;


    virtual int arun(const char*  ids[],const char*  tags[],const char* extend,QAParam* param,void * usrTag)  = 0;
    virtual int arun(const char* extend,QAParam* param,void * usrTag)  = 0;

};

}

#endif  //SC_QACHAIN_API_H
