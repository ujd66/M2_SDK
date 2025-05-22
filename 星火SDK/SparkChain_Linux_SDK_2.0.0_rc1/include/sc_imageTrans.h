#ifndef SC_IMAGETRANS_API_H
#define SC_IMAGETRANS_API_H

#include <string>
#include <vector>
#include "sc_type.h"
#include <memory>
using namespace std;

namespace SparkChain {

    class SPARKCHAIN_API BlockText {
    public:
        virtual string src() = 0;
        virtual string dst() = 0;
    };

    class SPARKCHAIN_API ImageTransResult {
    public:
        virtual string sid() = 0;
        virtual int status() = 0;
        virtual string itsImage() = 0;
        virtual string itsOutput() = 0;
        virtual vector<BlockText*> blockText() = 0;
    };

    class SPARKCHAIN_API ImageTransError {
    public:
        virtual string sid() = 0;
        virtual int code() = 0;
        virtual string errMsg() = 0;
    };

    class SPARKCHAIN_API ImageTransCallbacks {
    public:
        virtual void onResult(ImageTransResult* result, void* usrTag) = 0;
        virtual void onError(ImageTransError* error, void* usrTag) = 0;
    };

    class SPARKCHAIN_API ImageTrans {
    public:
        ImageTrans();
        ImageTrans(string ocrLanguage, string fromLanguage, string toLanguage);
        ~ImageTrans();

        void registerCallbacks(ImageTransCallbacks* cbs);

        void ocrLanguage(string ocrLanguage);
        void fromLanguage(string fromLanguage);
        void toLanguage(string toLanguage);
        void returnType(int returnType);

        int arun(const char* image, size_t imageSize, const char* encoding, void* usrTag = nullptr);

    private:
        class Impl;
        Impl* mImpl;
    };
}

#endif

