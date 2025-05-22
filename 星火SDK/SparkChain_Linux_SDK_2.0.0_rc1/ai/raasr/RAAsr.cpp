/*************************
 * 语音转写Demo
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
#include "../../include/sc_raAsr.h"//识别需要的头文件

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
string orderId  = "";

/*************************************SDK初始化参数**********************************************************/
char * APPID     = "APPID";                        //用户的APPID
char * APIKEY    = "APIKEY";                       //用户的APIKey
char * APISECRET = "APISECRET";                    //用户的APISecret
char * WORKDIR   = "./";                                    //SDK工作目录，要求有读写权限
int logLevel     = 100;                                     //日志等级。0：VERBOSE(日志最全)，1：DEBUG，2：INFO，3：WARN，4：ERROR，5：FATAL，100：OFF(关闭日志)
string RAASRAPIKEY = "您的RAASRAPIKEY，请从控制台https://console.xfyun.cn/services/lfasr 中查看SecretKey的值，填入此处";//您的RAASRAPIKEY，请从控制台https://console.xfyun.cn/services/lfasr 中查看SecretKey的值，填入此处。
/*************************************SDK初始化参数**********************************************************/



class RaAsrCallbacksImpl :public RaAsrCallbacks{
    void onResult(RaAsrResult* result, void* usrTag){
        int status            = result->status();//订单流程状态
        string orderResult    = result->orderResult();//转写结果(返回的是云端转写的原始结果，开发者自行解析)
        orderId               = result->orderId();//订单号
        int failType          = result->failType();//订单异常状态
        long originalDuration = result->originalDuration();//原始音频时长
        long realDuration     = result->realDuration();//真实音频时长
        int taskEstimateTime  = result->taskEstimateTime();//订单预估耗时
        //以下为翻译结果，开发者根据需求自行获取
        vector<RaAsrTransResult*> transResult = result->transResult();//获取翻译结果
        for (RaAsrTransResult * raAsrTransResult : transResult) {
            int bg               = raAsrTransResult->bg();//开始时间
            int ed               = raAsrTransResult->ed();//结束时间
            string dst           = raAsrTransResult->dst();//翻译结果
            string segId         = raAsrTransResult->segId();//段落序号
            vector<string> tags  = raAsrTransResult->tags();//标签
            vector<string> roles = raAsrTransResult->roles();//角色
        }
        
        if(status == 4){
            printf(GREEN "%s\n" RESET,orderResult.c_str());
            finish = true;
        }
    }
    void onError(RaAsrError* error, void* usrTag){
        string orderId = error->orderId();//订单号
        int errCode    = error->code();//错误码
        string errMsg  = error->errMsg();//错误信息
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



void run_raAsr(char* language){
    finish = false;
    RAASR raAsr(RAASRAPIKEY);//RTASR 初始化
    printf(YELLOW "转写语种为:%s\n" RESET,language);
    raAsr.language(language);//设置转写语种。cn:中文，en:英文
    RaAsrCallbacksImpl *mRAASRCallbacks = new RaAsrCallbacksImpl();  
    raAsr.registerCallbacks(mRAASRCallbacks);
    
    if(!strcmp(language,"cn")){
        audioPath  = "../../test_file/cn_test.pcm"; 
    }else{
        audioPath  = "../../test_file/en_test.pcm"; 
    }
    printf(YELLOW "转写音频路径为:%s\n" RESET,audioPath);
    orderId = "";
    int ret = raAsr.aRun(audioPath,"transfer");
    if(ret != 0){
        printf(RED "转写启动失败，错误码:%d\n" RESET,ret);
        finish = true;
    }else{
        printf(YELLOW "正在转写中...\n" RESET);
    }
    while (!finish)
    { // 等待结果返回退出
        sleep(1);        
    }
    printf(GREEN "转写演示完成,请继续输入指令:\n0:中文转写\n1:英文转写\n2:退出\n" RESET);

}


// void run_selectResult(){   
//     printf(GREEN "当前orderid是:%s\n" RESET,orderId.c_str());
//     if(!orderId.empty()){
//         RAASR raAsr(RAASRAPIKEY);//RTASR 初始化
//         RaAsrCallbacksImpl *mRAASRCallbacks = new RaAsrCallbacksImpl();  
//         raAsr.registerCallbacks(mRAASRCallbacks);
//         finish = false;
//         raAsr.aGetResult(orderId.c_str());
//         while (!finish)
//         { // 等待结果返回退出
//             sleep(1);        
//         }
//         printf(GREEN "查询演示完成,请继续输入指令:\n0:中文转写\n1:英文转写\n2:退出\n" RESET);
//     }else{
//         printf(RED "没有检测到订单号，请先体验转写，完成后再体验查询\n" RESET);
//         printf(GREEN "请继续输入指令:\n0:中文转写\n1:英文转写\n2:退出\n" RESET);
//     }
// }


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

    printf(GREEN "\n###############################\n" RESET);
    printf(GREEN "##语音转写:用户输入音频,转写出文本##\n" RESET);
    printf(GREEN "#################################\n" RESET);
    printf(GREEN "演示示例选择:\n0:中文转写\n1:英文转写\n2:退出\n" RESET);

    while(1)
    {
        scanf("%d", &choice);
        if(choice == 0){
            printf(GREEN "中文转写演示\n" RESET);
            run_raAsr("cn");
        }else if(choice == 1){
            printf(GREEN "英文转写演示\n" RESET);
            run_raAsr("en");
        }else{
            break;
        }
    }

exit:
	printf(RED "已退出演示 ...\n" RESET);
	uninitSDK(); //退出    
}
