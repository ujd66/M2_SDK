#ifndef SC_EUOCR_API_H
#define SC_EUOCR_API_H

#include <string>
#include <vector>
#include "sc_type.h"
#include <memory>
using namespace std;

namespace SparkChain {

    class SPARKCHAIN_API EuOCRResult {
    public:
        virtual string sid() = 0;
        virtual string ocrResult() = 0;
        virtual int status() = 0;
        /*virtual string encoding() = 0;
        virtual string compress() = 0;
        virtual string format() = 0;*/
    };

    class SPARKCHAIN_API EuOCRError {
    public:
        virtual string sid() = 0;
        virtual int code() = 0;
        virtual string errMsg() = 0;
    };

    class SPARKCHAIN_API EuOCRCallbacks {
    public:
        virtual void onResult(EuOCRResult* result, void* usrTag) = 0;
        virtual void onError(EuOCRError* error, void* usrTag) = 0;
    };

    class SPARKCHAIN_API EuOCR {
    public:
        EuOCR();
        ~EuOCR();

        void registerCallbacks(EuOCRCallbacks* cbs);

        void language(string language);
        void encoding(string encoding);
        void compress(string compress);
        void format(string format);

        int arun(string image, string imageEncoding, void* usrTag = nullptr);

    private:
        class Impl;
        Impl* mImpl;
    };
}

#endif

