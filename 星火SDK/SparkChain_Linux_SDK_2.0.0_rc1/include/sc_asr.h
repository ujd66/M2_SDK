#ifndef SC_ASR_API_H
#define SC_ASR_API_H

#include <string>
#include <vector>
#include "sc_type.h"
using namespace std;

namespace SparkChain {

class SPARKCHAIN_API Segment {
public:
    virtual int score() = 0;
    virtual string text() = 0;
};

class SPARKCHAIN_API Transcription {
public:
    virtual int index() = 0;
    virtual vector<Segment *> segments() = 0;
};
class SPARKCHAIN_API Vad {
public:
    virtual int begin() = 0;
    virtual int end() = 0;
};

class SPARKCHAIN_API ASRResult {
public:
    virtual vector<Transcription *> transcriptions() = 0;
    virtual vector<Vad *> vads() = 0;
    virtual int begin() = 0;
    virtual int end() = 0;
    virtual string sid() = 0;
    virtual string bestMatchText() = 0;
    virtual int status() = 0;
};

class SPARKCHAIN_API ASRError {
public: 
    virtual string sid() = 0;
    virtual int code() = 0;
    virtual string errMsg() = 0;
};

class SPARKCHAIN_API ASRCallbacks {
public:
    virtual void onResult(ASRResult * result, void * usrTag) = 0;
    virtual void onError(ASRError * error,void * usrTag) = 0;
};
class SPARKCHAIN_API AudioAttributes {
private:
    int mSampleRate = 16000;
    string mEncoding = "raw";
    int mChannels = 1;
    int mBitepth = 16;
    int mFrameSize = 0;
   
public:
    AudioAttributes(){}
    AudioAttributes(const AudioAttributes & attributes);
    
    void setSampleRate(int sampleRate);

    void setEncoding(const string & encoding);

    void setChannels(int channels);

    void setBitDepth(int bitdepth);

    void setFrameSize(int framesize);

    int getSampleRate();

    string getEncoding();

    int getChannels();

    int getBitDepth();

    int getFrameSize();
};

class SPARKCHAIN_API ASR {
public: 
    ASR();
    ASR(string language, string domain, string accent);
    ~ASR();

    void registerCallbacks(ASRCallbacks * cbs);

    void language(string language);
    void domain(string domain);
    void accent(string accent);

    void vadEos(int vadEos);
    void dwa(string dwa);
    void ptt(bool enable);
    void rlang(string rlang);
    void vinfo(bool vinfo);
    void numnum(bool enable);

    // 大模型识别参数
    void vgap(int vgap);
    void proc(bool proc);
    void smth(bool smth);

    void ln(string ln);

    // 普通识别参数
    void nbest(int nbest);
    void wbest(int wbest);
    void pd(string pd);
    void speexSize(int speexSize);

    int start(const AudioAttributes & attributes,void * usrTag = nullptr);
    int write(const char * data, size_t len);
    int stop(bool immediate = false);

    private:
        class Impl;
        Impl * mImpl;
};
}

#endif

