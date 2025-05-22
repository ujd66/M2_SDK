#ifndef SC_RTASR_API_H
#define SC_RTASR_API_H

#include <string>
#include <vector>
#include "sc_type.h"
#include <memory>
using namespace std;

namespace SparkChain {

    class SPARKCHAIN_API RtTransResult {
    public:
        virtual ~RtTransResult() {}
        virtual int status() = 0;
        virtual string src() = 0;
        virtual string dst() = 0;
    };

    class SPARKCHAIN_API RtAsrResult {
    public:
        virtual ~RtAsrResult() {}
        virtual int status() = 0;
        virtual string sid() = 0;
        virtual string rawResult() = 0;
        virtual string data() = 0;
        virtual RtTransResult* transResult() = 0;
    };

    class SPARKCHAIN_API RtAsrError {
    public:
        virtual string sid() = 0;
        virtual int code() = 0;
        virtual string errMsg() = 0;
    };

    class SPARKCHAIN_API RtAsrCallbacks {
    public:
        virtual void onResult(RtAsrResult* result, void* usrTag) = 0;
        virtual void onError(RtAsrError* error, void* usrTag) = 0;
    };

    class SPARKCHAIN_API RTASR {
    public:
        RTASR(const string apiKey);
        //RTASR(string fromlanguage, string tolanguage);
        ~RTASR();

        void registerCallbacks(RtAsrCallbacks* cbs);

        void lang(string lang);
        void transType(string transType);
        void transStrategy(int transStrategy);
        void targetLang(string targetLang);
        void punc(string punc);
        void pd(string pd);
        void vadMdn(int vadMdn);
        void roleType(int roleType);
        void engLangType(int engLangType);

        int start(void* usrTag = nullptr);
        int write(const char* data, size_t len);
        int stop();

    private:
        class Impl;
        Impl* mImpl;
    };
}

#endif

