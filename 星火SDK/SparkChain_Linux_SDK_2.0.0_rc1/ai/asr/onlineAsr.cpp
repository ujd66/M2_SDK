/*************************
 * 语音听写Demo
 * create by wxw
 * 2024-12-17
 * ***********************/
#include <iostream>
#include <string>
#include <atomic>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <regex>
#include "../../include/sparkchain.h"
#include "../../include/sc_asr.h"//识别需要的头文件

#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define RESET "\033[0m"

using namespace SparkChain;
using namespace std;

// async status tag
static atomic_bool finish(false);
// result cache
string final_result = "";
static FILE * file = nullptr;
char* audioPath = "../../test_file/cn.pcm";

/*************************************SDK初始化参数**********************************************************/
char * APPID     = "APPID";                        //用户的APPID
char * APIKEY    = "APIKEY";                       //用户的APIKey
char * APISECRET = "APISECRET";                    //用户的APISecret
char * WORKDIR   = "./";                                    //SDK工作目录，要求有读写权限
int logLevel     = 100;                                     //日志等级。0：VERBOSE(日志最全)，1：DEBUG，2：INFO，3：WARN，4：ERROR，5：FATAL，100：OFF(关闭日志)
/*************************************SDK初始化参数**********************************************************/

//识别监听回调
class ASRCallbacksImpl : public ASRCallbacks {
    void onResult(ASRResult * result, void * usrTag) override {
        int begin = -1;
        int end = -1;
        string word = "";
        string asrResult = result->bestMatchText();//解析识别结果
        int status       = result->status();//解析结果返回状态
        string sid       = result->sid();//解析sid

        //以下信息需要开发者根据自身需求，如无必要，可不需要解析执行。
        vector<Vad *> vads = result->vads();//解析vad结构体
        for (Vad * v : vads) {
            begin = v->begin();//解析vad前端点
            end   = v->end();//解析vad后端点   
            printf(YELLOW "vad结果:begin:%d,end:%d\n" RESET, begin,end);        
        }
        vector<Transcription *> transcriptions = result->transcriptions();//解析具体结果信息
        for (auto item : transcriptions) {
            vector<Segment *> segs = item->segments();//解析中文分词结构体
            for (auto seg : segs) {
                word = seg->text();//解析词语
                printf(YELLOW "分词结果:%s\n" RESET, word.c_str());
            }
        }
        printf(GREEN "识别结果: %s status = %d\n" RESET, asrResult.c_str(),status);
        if (status == 2)
        {
            finish = true;
        } 

    }
    void onError(ASRError * error,void * usrTag) override {
        int errCode   = error->code();//错误码
        string errMsg = error->errMsg();//错误信息
        string sid    = error->sid();//本次交互的sid

        printf(RED "请求出错,错误码: %d,错误信息:%s,交互sid:%s\n" RESET, errCode, errMsg, sid);
        finish = true; 
    }
};



/***
 * SDK初始化
 * ***/
int initSDK()
{
    SparkChainConfig *config = SparkChainConfig::builder();
    config->appID(APPID)        // 你的appid
        ->apiKey(APIKEY)        // 你的apikey
        ->apiSecret(APISECRET)   // 你的apisecret
        ->workDir(WORKDIR)
        ->logLevel(logLevel); 
    int ret = SparkChain::init(config);
    return ret;
}

static void run_asr(char* language){
    /***************
     * 构建ASR实例
     * 中文识别：language=zh_cn;domain=iat;accent=mandarin
     * 英文识别：language=en_us;domain=;accent=mandarin
     * *****************/
    finish = false;

    ASR asr(language,"iat","mandarin");
    asr.vinfo(true);//句子级别帧对齐:给出一次会话中，子句的vad边界信息。 false:不返回vad信息 true:返回vad信息

    ASRCallbacksImpl *mASRCallbacks = new ASRCallbacksImpl();
    asr.registerCallbacks(mASRCallbacks);//注册监听回调

    if(!strcmp(language,"zh_cn")){
        audioPath  = "../../test_file/cn.pcm"; 
    }else{
        audioPath  = "../../test_file/en.pcm"; 
    }

    printf(YELLOW "\n开始识别,音频路径:%s\n" RESET,audioPath);
    SparkChain::AudioAttributes attr;
    attr.setSampleRate(16000);  //输入音频采样率:16K:16000,8K:8000
    attr.setEncoding("raw");    //输入音频的编码格式:raw：pcm音频（支持单声道的pcm）,speex：speex压缩后的音频（8k） speex-wb：speex压缩后的音频（16k） 请注意压缩前也必须是采样率16k或8k单声道的pcm。 lame：mp3格式（仅中文普通话和英文支持，方言及小语种暂不支持）
    attr.setChannels(1);        //输入音频的声道 1:单声道(默认) 2:双声道
    asr.start(attr);

    /***********************送音频数据 start******************************/
    FILE * file = fopen(audioPath, "r");
    fseek(file, 0, SEEK_END);
    size_t fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);
    char * data = new char[fileLen];
    fread(data, 1, fileLen, file);
    const int per_frame_size = 1280*8;
    size_t readLen = 0;
    while (readLen < fileLen) {
        size_t curSize = per_frame_size;
        if (readLen + per_frame_size > fileLen) {
            curSize = fileLen - readLen;
        }

        asr.write(data + readLen,curSize);
        readLen += curSize;
        usleep(40 * 1000);
    }
    asr.stop();
    /***********************送音频数据 end******************************/
    int times = 0;
    while (!finish)
    { // 等待结果返回退出
        sleep(1);
        if (times++ > 10) // 等待十秒如果没有最终结果返回退出
            break;
    }
    printf(GREEN "识别演示完成,请继续输入指令:\n0:中文识别\n1:英文识别\n2:退出\n" RESET);
}


void uninitSDK()
{
    // SDK逆初始化
    SparkChain::unInit();
}

int main(int argc, char const *argv[])
{
    int choice = 1; //演示模式选择。

    /* SDK初始化,初始化仅需全局初始化一次。*/
    int ret = initSDK();

    if (ret != 0)
    {
        printf(RED "\nSDK初始化失败!错误码:%d" RESET,ret);
        goto exit; //SDK初始化失败，退出
    }

    printf(GREEN "\n#################################\n" RESET);
    printf(GREEN "##在线识别:用户输入音频,在线识别结果##\n" RESET);
    printf(GREEN "###################################\n" RESET);
    printf(GREEN "演示示例选择:\n0:中文识别\n1:英文识别\n2:退出\n" RESET);

    while(1)
    {
        scanf("%d", &choice);
        if(choice == 0){
            printf(GREEN "中文识别演示\n" RESET);
            run_asr("zh_cn");
        }else if(choice == 1){
            printf(GREEN "英文识别演示\n" RESET);
            run_asr("en_us");
        }else{
            break;
        }
    }

exit:
	printf(RED "已退出演示 ...\n" RESET);
	uninitSDK(); //退出    
}
