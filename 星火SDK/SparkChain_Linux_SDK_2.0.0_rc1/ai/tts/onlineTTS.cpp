#include <iostream>
/*************************
 * 在线合成Demo
 * create by wxw
 * 2024-12-17
 * ***********************/
#include <string>
#include <atomic>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include "../../include/sparkchain.h"
#include "../../include/sc_tts.h"


#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define RESET "\033[0m"

using namespace SparkChain;
using namespace std;

// async status tag
static atomic_bool finish(false);

static FILE * file = nullptr;
int choice = 1; //演示模式选择。
char* audioPath = "./audio.pcm";

/*************************************SDK初始化参数**********************************************************/
char *APPID     = "APPID";                             // 用户的APPID
char *APIKEY    = "APIKEY";    // 用户的APIKey
char *APISECRET = "APISECRET"; // 用户的APISecret
char *WORKDIR   = "./";                                 // SDK工作目录，要求有读写权限
int logLevel    = 100;                                   // 日志等级。0：VERBOSE(日志最全)，1：DEBUG，2：INFO，3：WARN，4：ERROR，5：FATAL，100：OFF(关闭日志)
/*************************************SDK初始化参数**********************************************************/

class onlineTTSCallbacks : public TTSCallbacks {
    void onResult(TTSResult * result,void * usrTag) override {
        const char * data = result->data();//音频数据
        int len           = result->len();//音频数据长度
        int status        = result->status();//数据状态
        string ced        = result->ced();//进度
        string sid        = result->sid();//sid  

        if (file == nullptr) {
            file = fopen(audioPath, "a");
        }
        fwrite(data, 1, len, file);
        if (status == 2) {
            fclose(file);
            file = nullptr;           
            printf(YELLOW "音频合成完成,音频保存路径: %s\n" RESET, audioPath);
            finish = true;
        }
    }

    void onError(TTSError * error, void * usrTag) override {        
        int errCode   = error->code();//错误码
        string errMsg = error->errMsg();//错误信息
        string sid    = error->sid();//sid
        printf(RED "请求出错,错误码: %d,错误信息:%s,交互sid:%s\n" RESET, errCode, errMsg.c_str(), sid.c_str());    
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

void run_TTS(){   
    /***********
     * 构造参数:
     * vcn:发音人
     * xiaoyan:讯飞小燕
     * aisjiuxu:讯飞许久
     * aisxping:讯飞小萍
     * aisjinger:讯飞小婧
     * aisbabyxu:讯飞许小宝
     * ************/    
    OnlineTTS tts("xiaoyan");
    onlineTTSCallbacks cbs{};
    tts.registerCallbacks(&cbs);

    char input[1024];
    
    while(1){
        printf(GREEN "请输入要合成的文本，如要退出请输入q。\n" RESET);
        if(fgets(input, sizeof(input), stdin) == NULL){
            printf(RED "读取输入时出错\n" RESET);
            break;
        }
        // 去掉换行符
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }
        // 判断是否输入了'q'
        if (!strcmp(input, "q")) {
            break;
        }else if(!strcmp(input, "")){
            continue;
        }
        finish = false;
        printf(YELLOW "正在合成中,请稍后... \n" RESET);

        int ret = tts.arun(input);//arun方法设计存在一个问题，即自定义标识无法设置成int型
        if (ret != 0)
        {
            printf(RED "合成请求失败,错误码: %d\n" RESET, ret);
            finish = true;
            break;
        }
        int times = 0;
        while (!finish)
        { // 等待结果返回退出
            sleep(1);
            if (times++ > 20) // 等待十秒如果没有最终结果返回退出
                break;
        }
    }

}


void uninitSDK()
{
    // SDK逆初始化
    SparkChain::unInit();
}

int main(int argc, char const *argv[])
{
    /* SDK初始化,初始化仅需全局初始化一次。*/
    int ret = initSDK();

    if (ret != 0)
    {
        printf(RED "\nSDK初始化失败!错误码:%d\n" RESET,ret);
        goto exit; //SDK初始化失败，退出
    }

    printf(GREEN "\n####################################\n" RESET);
    printf(GREEN "##在线合成:用户输入文本，在线合成出音频##\n" RESET);
    printf(GREEN "######################################\n" RESET);
    printf(RED "当前为默认发音人xiaoyan。如需体验其他发音人，请先获得授权，然后在demo中修改发音人体验。\n" RESET);
    run_TTS();
exit:
	printf(RED "已退出演示 ...\n" RESET);
	uninitSDK(); //退出    
}
