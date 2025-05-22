#ifndef SC_OCR_API_H
#define SC_OCR_API_H

#include <string>
#include <vector>
#include "sc_type.h"
#include <memory>
using namespace std;

namespace SparkChain {

class SPARKCHAIN_API Line {
public:
    virtual int angle() = 0;
    virtual vector<vector<int>> char_centers() = 0;
    virtual vector<vector<int>> char_polygons() = 0;
    virtual vector<double> char_score() = 0;
    virtual vector<int> position() = 0;
    virtual int property() = 0;
    virtual double score() = 0;
    virtual string text() = 0;
};

class SPARKCHAIN_API OCRResult {
public:
    virtual string sid() = 0;
    virtual int status() = 0;

    virtual int image_angle() = 0;
    virtual vector<Line*> lines() = 0;
    virtual vector<string> property_map() = 0;
    virtual int rotated_image_height() = 0;
    virtual int rotated_image_width() = 0;
    virtual string whole_text() = 0;
};

class SPARKCHAIN_API OCRError {
public: 
    virtual string sid() = 0;
    virtual int code() = 0;
    virtual string errMsg() = 0;
};

class SPARKCHAIN_API OCRCallbacks {
public:
    virtual void onResult(OCRResult * result, void * usrTag) = 0;
    virtual void onError(OCRError * error,void * usrTag) = 0;
};

class SPARKCHAIN_API OCR {
public: 
    OCR();
    OCR(string encoding, string compress, string format);
    ~OCR();

    void registerCallbacks(OCRCallbacks * cbs);

    void encoding(string encoding);
    void compress(string compress);
    void format(string format);

    int arun(const char* imagepath, void* usrTag = nullptr);
    int arundata(const char* imagedata,size_t size,void* usrTag = nullptr);

    private:
        class Impl;
        Impl * mImpl;
};
}

#endif

