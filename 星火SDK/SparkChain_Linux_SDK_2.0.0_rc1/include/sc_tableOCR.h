#ifndef SC_TABLEOCR_API_H
#define SC_TABLEOCR_API_H

#include <string>
#include <vector>
#include "sc_type.h"
#include <memory>
using namespace std;

namespace SparkChain {

    class SPARKCHAIN_API TableOCRResult {
    public:
        virtual string sid() = 0;
        virtual vector<string> tables() = 0;
        virtual string resultAll() = 0;
    };

    class SPARKCHAIN_API TableOCRError {
    public:
        virtual string sid() = 0;
        virtual int code() = 0;
        virtual string errMsg() = 0;
    };

    class SPARKCHAIN_API TableOCRCallbacks {
    public:
        virtual void onResult(TableOCRResult* result, void* usrTag) = 0;
        virtual void onError(TableOCRError* error, void* usrTag) = 0;
    };

    class SPARKCHAIN_API TableOCR {
    public:
        TableOCR();
        ~TableOCR();

        void registerCallbacks(TableOCRCallbacks* cbs);

        void resultEncoding(string encoding);
        void resultCompress(string compress);
        void resultFormat(string format);

        int arun(const char* image, size_t imageSize, const char* encoding, void* usrTag = nullptr);

    private:
        class Impl;
        Impl* mImpl;
    };
}

#endif

