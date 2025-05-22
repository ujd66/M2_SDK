/*************************
 * 实时语音转写Demo
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
#include "../../include/sc_rtasr.h"//识别需要的头文件

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
char* audioPath = "../../test_file/cn_test.pcm";

/*************************************SDK初始化参数**********************************************************/
char * APPID     = "APPID";                        //用户的APPID
char * APIKEY    = "APIKEY";                       //用户的APIKey
char * APISECRET = "APISECRET";                    //用户的APISecret
char * WORKDIR   = "./";                                    //SDK工作目录，要求有读写权限
int logLevel     = 100;                                     //日志等级。0：VERBOSE(日志最全)，1：DEBUG，2：INFO，3：WARN，4：ERROR，5：FATAL，100：OFF(关闭日志)
string RTASRAPIKEY = "您的RTASRAPIKEY，请从控制台 https://console.xfyun.cn/services/rta 中查看APIKey的值填入此处。";//您的RTASRAPIKEY，请从控制台 https://console.xfyun.cn/services/rta 中查看APIKey的值填入此处。
/*************************************SDK初始化参数**********************************************************/


class RtAsrCallbacksImpl :public RtAsrCallbacks{
    void onResult(RtAsrResult* result, void* usrTag){
        int status       = result->status();//数据状态。0:此次返回翻译结果;1:流式结果;2:子句plain结果;3:end结束
        string sid       = result->sid();//本次交互的sid
        string rawResult = result->rawResult();//云端下发的原始json结果
        string data      = result->data();//转写结果
        string src       = result->transResult()->src();//翻译源文本
        string dst       = result->transResult()->dst();//翻译结果

        printf(GREEN "%s\n" RESET,data.c_str());
        if(status == 3){
            finish = true;
        }
    }
    void onError(RtAsrError* error, void* usrTag){
        string sid    = error->sid();//本次交互的sid
        int code      = error->code();//错误码
        string errMsg = error->errMsg();//错误信息
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



void run_rtasr(char* language){
    finish = false;
    RTASR rtasr(RTASRAPIKEY);//RTASR 初始化
    printf(YELLOW "转写语种为:%s\n" RESET,language);
    rtasr.lang(language);//设置转写语种。cn:中文，en:英文
    RtAsrCallbacksImpl *mRTASRCallbacks = new RtAsrCallbacksImpl();  
    rtasr.registerCallbacks(mRTASRCallbacks);
    
    //实时转写支持同时出翻译结果。翻译功能需在控制台开启。以下参数当开启翻译后生效。
    // rtasr.transType("normal");//normal表示普通翻译
    // rtasr.transStrategy(2);//翻译策略2:返回中间过程中的结果。其他策略参考集成文档
    // rtasr.targetLang("en");//翻译目标语种。en:英文。其他语种参考集成文档
    //其他功能参数demo不一一列举，如有需要参考集成文档。
    if(!strcmp(language,"cn")){
        audioPath  = "../../test_file/cn_test.pcm"; 
    }else{
        audioPath  = "../../test_file/en_test.pcm"; 
    }

    int ret = rtasr.start();
    if(ret != 0 ){
        printf(RED "转写启动失败，错误码:%d\n" RESET,ret);
        printf(GREEN "转写演示完成,请继续输入指令:\n0:中文转写\n1:英文转写\n2:退出\n" RESET);
        finish = true;
        return;
    }else{
        printf(YELLOW "转写音频路径为:%s\n" RESET,audioPath);
        printf(YELLOW "正在转写中...\n" RESET);
    }   
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

        rtasr.write(data + readLen,curSize);
        readLen += curSize;
        usleep(40 * 1000);
    }
    rtasr.stop();
    /***********************送音频数据 end******************************/
    int times = 0;
    while (!finish)
    { // 等待结果返回退出
        sleep(1);
        if (times++ > 10) // 等待十秒如果没有最终结果返回退出
            break;
    }
    printf(GREEN "转写演示完成,请继续输入指令:\n0:中文转写\n1:英文转写\n2:退出\n" RESET);

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

    printf(GREEN "\n###################################\n" RESET);
    printf(GREEN "##实时转写:用户输入音频,实时转写出文本##\n" RESET);
    printf(GREEN "#####################################\n" RESET);
    printf(GREEN "演示示例选择:\n0:中文转写\n1:英文转写\n2:退出\n" RESET);

    while(1)
    {
        scanf("%d", &choice);
        if(choice == 0){
            printf(GREEN "中文转写演示\n" RESET);
            run_rtasr("cn");
        }else if(choice == 1){
            printf(GREEN "英文转写演示\n" RESET);
            run_rtasr("en");
        }else{
            break;
        }
    }

exit:
	printf(RED "已退出演示 ...\n" RESET);
	uninitSDK(); //退出    
}
