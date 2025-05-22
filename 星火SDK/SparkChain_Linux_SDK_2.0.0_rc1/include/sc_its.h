#ifndef SC_ITS_API_H
#define SC_ITS_API_H

#include <string>
#include <vector>
#include "sc_type.h"
#include <memory>
using namespace std;

namespace SparkChain {

enum TransType {
    ITRANS = 0, //讯飞翻译
    NIUTRANS,   //小牛翻译
    ITRANS_SG   //讯飞翻译(新加坡)
};

class SPARKCHAIN_API Trans_Result {
public:
    virtual string src() = 0;
    virtual string dst() = 0;
};

class SPARKCHAIN_API ITSResult {
public:
    virtual string sid() = 0;
    virtual Trans_Result* trans_result() = 0;
    virtual int status() = 0;
    virtual string from() = 0;
    virtual string to() = 0;
};

class SPARKCHAIN_API ITSError {
public: 
    virtual string sid() = 0;
    virtual int code() = 0;
    virtual string errMsg() = 0;
};

class SPARKCHAIN_API ITSCallbacks {
public:
    virtual void onResult(ITSResult * result, void * usrTag) = 0;
    virtual void onError(ITSError * error,void * usrTag) = 0;
};

class SPARKCHAIN_API ITS {
public: 
    ITS();
    ITS(TransType type);

    ITS(string fromlanguage, string tolanguage, TransType type = ITRANS);
    ~ITS();

    void registerCallbacks(ITSCallbacks * cbs);

    void fromlanguage(string fromlanguage);
    void tolanguage(string tolanguage);

    int arun(const char* txt, void* usrTag = nullptr);

    private:
        class Impl;
        Impl * mImpl;
};
}

#endif

