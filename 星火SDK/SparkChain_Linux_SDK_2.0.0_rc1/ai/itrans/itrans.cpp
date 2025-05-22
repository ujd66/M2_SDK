/*************************
 * 在线翻译Demo
 * create by wxw
 * 2024-12-17
 * ***********************/
#include "../../include/sparkchain.h"
#include <iostream>
#include <string>
#include <atomic>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include "../../include/sc_its.h"


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
int transType = 0;//翻译模式选择。

/*************************************SDK初始化参数**********************************************************/
char * APPID     = "APPID";                        //用户的APPID
char * APIKEY    = "APIKEY";                       //用户的APIKey
char * APISECRET = "APISECRET";                    //用户的APISecret
char * WORKDIR   = "./";                                    //SDK工作目录，要求有读写权限
int logLevel     = 100;                                      //日志等级。0：VERBOSE(日志最全)，1：DEBUG，2：INFO，3：WARN，4：ERROR，5：FATAL，100：OFF(关闭日志)
/*************************************SDK初始化参数**********************************************************/

class ITSCallbacksImpl:public SparkChain::ITSCallbacks {
    void onResult(ITSResult * result, void * usrTag) override {
        //解析获取的交互结果，示例展示所有结果获取，开发者可根据自身需要，选择获取。
        string sid  = result->sid();                  //本次交互的sid
        int status  = result->status();               //数据状态。3:onece(一次性)
        string from = result->from();                 //源语种
        string to   = result->to();                   //目标语种
        string src  = result->trans_result()->src();  //源文本
        string dst  = result->trans_result()->dst();  //翻译结果
        if(status == 3){
            printf(YELLOW "翻译结果:%s\n" RESET, dst.c_str());//翻译结果乱码，周一查看原因
            finish = true;
        }
        
    }

    void onError(ITSError * error,void * usrTag) override {
        string sid    = error->sid();                  //本次交互的sid
        int code      = error->code();                 //错误码
        string errMsg = error->errMsg();               //错误信息

        printf(YELLOW "翻译出错了!错误码:%d,错误信息:%s,sid:%s" RESET, code,errMsg.c_str(),sid.c_str());
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

void run_Itrans(char* from, char* to, TransType type){
    ITSCallbacksImpl cbs{};
    /*************
     * 构造方法形参解释：
     * from:翻译源语种
     * to:翻译目标语种
     * type:翻译类型:
     *  ITRANS:讯飞翻译;
     *  NIUTRANS:小牛翻译;
     *  ITRANS_SG:海外翻译。海外翻译服务器部署在国外，使用前需确保能访问国外网络。否则可能会报网络问题。
     * ***************/
    ITS its(from,to,type);
    its.registerCallbacks(&cbs);
    // its.fromlanguage("cn"); //设置源语种。初始化设置过语种后这里可不设置。demo仅展示调用方法。
    // its.tolanguage("en");//设置目标语种。初始化设置过语种后这里可不设置。demo仅展示调用方法。
    char input[1024];
    printf(GREEN "请输入翻译源文本,如需退出请输入q: \n" RESET);
    while(1){        
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
        printf(YELLOW "正在翻译中,请稍后...%s \n" RESET,input);
        finish = false;
        int ret = its.arun(input);
        if (ret != 0)
        {
            printf(RED "翻译请求失败,错误码: %d\n" RESET, ret);
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
        printf(GREEN "请输入翻译源文本,如需退出请输入q: \n" RESET);
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

    printf(GREEN "\n#########################################\n" RESET);
    printf(GREEN "##在线翻译:用户输入源文本，在线翻译成目标文本##\n" RESET);
    printf(GREEN "###########################################\n" RESET);
    while(1)
    {
        printf(GREEN "演示示例选择:\n0:讯飞翻译\n1:小牛翻译\n2:退出\n" RESET);
        scanf("%d", &choice);
        if(choice == 0){
            printf(GREEN "您已选择讯飞翻译。\n" RESET);
            while(1){
                printf(GREEN "请选择翻译方向:\n0:中译英\n1:英译中\n2:退出\n" RESET);
                scanf("%d", &transType);
                if(transType == 0){
                    run_Itrans("cn","en",ITRANS);
                }else if(transType == 1){
                    run_Itrans("en","cn",ITRANS);
                }else{
                    break;
                }
            }
        }else if(choice == 1){
            printf(GREEN "您已选择小牛翻译。\n" RESET);
            while(1){
                printf(GREEN "请选择翻译方向:\n0:中译英\n1:英译中\n2:退出\n" RESET);
                scanf("%d", &transType);
                if(transType == 0){
                    run_Itrans("cn","en",NIUTRANS);
                }else if(transType == 1){
                    run_Itrans("en","cn",NIUTRANS);
                }else{
                    break;
                }
            }
        }else{
            break;
        }
    }

exit:
	printf(RED "已退出演示 ...\n" RESET);
	uninitSDK(); //退出    
}
