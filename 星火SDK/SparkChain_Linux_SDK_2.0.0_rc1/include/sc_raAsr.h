#ifndef RAASR_H
#define RAASR_H

#include <string>
#include <vector>
#include "sc_type.h"

using namespace std;

namespace SparkChain {

class SPARKCHAIN_API RaAsrTransResult {
public:
    virtual ~RaAsrTransResult(){}
    virtual string segId() = 0;
    virtual string dst() = 0;
    virtual int bg() = 0;
    virtual int ed() = 0;
    virtual vector<string> tags() = 0;
    virtual vector<string> roles() = 0;
};

class SPARKCHAIN_API RaAsrResult {
public:
    virtual ~RaAsrResult(){}
    virtual int code() = 0;
    virtual string descInfo() = 0;
    virtual string orderResult() = 0;
    virtual vector<RaAsrTransResult*> transResult() = 0;
    virtual string orderId() = 0;
    virtual int failType() = 0;
    virtual int status() = 0;
    virtual long originalDuration() = 0;
    virtual long realDuration() = 0;
    virtual long expireTime() = 0;
    virtual int taskEstimateTime() = 0;
};

class SPARKCHAIN_API RaAsrError {
public: 
    virtual string orderId() = 0;
    virtual int code() = 0;
    virtual string errMsg() = 0;
    virtual int failType() = 0;
};

class SPARKCHAIN_API RaAsrCallbacks {
public:
    virtual void onResult(RaAsrResult* result, void* usrTag) = 0;
    virtual void onError(RaAsrError * error, void* usrTag) = 0;
};


class SPARKCHAIN_API RAASR
{
public:
    RAASR(const string apiKey);
    ~RAASR();

    void registerCallbacks(RaAsrCallbacks * cbs);

    void language(string language);
    void hotWord(string hotWord);
    void candidate(int candidate);
    void roleType(int roleType);
    void roleNum(int roleNum);
    void pd(string pd);
    void audioMode(string audioMode);
    void audioUrl(string audioUrl);
    void standardWav(int standardWav);
    void languageType(int languageType);
    void trackMode(int trackMode);
    void transLanguage(string transLanguage);
    void transMode(int transMode);
    void engSegMax(int engSegMax);
    void engSegMin(int engSegMin);
    void engSegWeight(float engSegWeight);
    void engSmoothproc(bool engSmoothproc);
    void engColloqproc(bool engColloqproc);
    void engVadMdn(int engVadMdn);
    void engVadMargin(int engVadMargin);
    void engRlang(int engRlang);

    //void waitResult(int waitResult);

    int aRun(string fileName, string resultType = "transfer", void* usrTag = nullptr);
    int aGetResult(string orderId, string resultType= "transfer", void* usrTag = nullptr);
    //int getResultOnce(string& resultOut, string orderId, string resultType = "transfer", void* usrTag = nullptr);

private:
    class Impl;
    Impl *mImpl;
};

}

#endif