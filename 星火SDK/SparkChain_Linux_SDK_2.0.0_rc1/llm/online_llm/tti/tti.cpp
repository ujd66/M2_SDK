/*************************
 * 图片生成Demo
 * create by wxw
 * 2024-12-17
 * ***********************/
#include "../../../include/sparkchain.h"
#include <iostream>
#include <string>
#include <atomic>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <regex>


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

/*************************************SDK初始化参数**********************************************************/
char * APPID     = "APPID";                        //用户的APPID
char * APIKEY    = "APIKEY";                       //用户的APIKey
char * APISECRET = "APISECRET";                    //用户的APISecret
char * WORKDIR   = "./";                                    //SDK工作目录，要求有读写权限
int logLevel     = 100;                                     //日志等级。0：VERBOSE(日志最全)，1：DEBUG，2：INFO，3：WARN，4：ERROR，5：FATAL，100：OFF(关闭日志)
/*************************************SDK初始化参数**********************************************************/

//文本交互，图片生成，图片理解监听回调
class SparkCallbacks : public LLMCallbacks
{
    void onLLMResult(LLMResult *result, void *usrContext)
    {
        //解析获取的结果，示例展示所有结果获取，开发者可根据自身需要，选择获取。
        const char* content  = result->getContent();//获取返回结果
        int contentLen       = result->getContentLen();//获取返回结果长度
        SparkChain::LLMResult::ContentType type = result->getContentType();//获取返回结果类型,TEXT:返回结果为文本,IMAGE:返回结果为图片
        int status           = result->getStatus();//返回结果状态
        const char* role     = result->getRole();//获取角色信息
        const char* sid      = result->getSid();//获取本次会话的sid
        int completionTokens = result->getCompletionTokens();//获取回答的Token大小
        int promptTokens     = result->getPromptTokens();//包含历史问题的总Tokens大小
        int totalTokens      = result->getTotalTokens();//promptTokens和completionTokens的和，也是本次交互计费的Tokens大小

        if (result->getContentType() == LLMResult::IMAGE) {
            FILE * ptr = fopen("./asyncpic.jpg", "w");  
            fwrite(content, contentLen, 1, ptr);
            fclose(ptr);
            printf(YELLOW "您的图片已生成，存放路径：./asyncpic.jpg\n" RESET);
            finish = true;
        } 
    }

    void onLLMEvent(LLMEvent *event, void *usrContext)
    {
        int eventId          = event->getEventID();//获取事件id
        const char* eventMsg = event->getEventMsg();//获取事件信息
        const char* sid      = event->getSid();//获取交互sid
    }

    void onLLMError(LLMError *error, void *usrContext)
    {
        int errCode        = error->getErrCode();//获取错误码
        const char* errMsg = error->getErrMsg();//获取错误信息
        const char* sid    = error->getSid();//获取交互sid
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


void run_imageGeneration_Async(){
    LLMConfig *llmConfig = LLMConfig::builder();   
    llmConfig->maxToken(2048); //回答的tokens的最大长度,取值范围1-4096，默认2048。其他配置参数请参考集成文档
    /********************
     * 构建图片生成的LLM，入参为要生成的图片尺寸。当前支持：
     * 512*512
     * 640*360
     * 640*480
     * 640*640
     * 680*512
     * 512*680
     * 768*768
     * 720*1280
     * 1280*720
     * 1024*1024
     * **************************/
    LLM * llm = LLMFactory::imageGeneration(512,512,llmConfig);

    finish = false;
    // 注册监听回调
    SparkCallbacks *cbs = new SparkCallbacks();
    llm->registerLLMCallbacks(cbs);

    char input[1024];
    printf(GREEN "请输入图片主题,如需退出请输入q: \n" RESET);
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
        printf(YELLOW "图片生成中,请稍后... \n" RESET);
        finish = false;
        int ret = llm->arun(input);
        if (ret != 0)
        {
            printf(RED "图片生成失败,错误码: %d\n" RESET, ret);
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
        printf(GREEN "请输入图片主题,如需退出请输入q: \n" RESET);
    }
    // 运行结束，释放实例  
    if (llm != nullptr)
    {
        LLM::destroy(llm);
    }
    if (cbs != nullptr)
        delete cbs;
    printf(GREEN "异步演示完成,请继续输入指令:\n0:同步生图\n1:异步生图\n2:退出\n" RESET);
}

void run_imageGeneration_Sync(){

    LLMConfig *llmConfig = LLMConfig::builder();
    llmConfig->maxToken(2048); //回答的tokens的最大长度,取值范围1-4096，默认2048。其他配置参数请参考集成文档

    /********************
     * 构建图片生成的LLM，入参为要生成的图片尺寸。当前支持：
     * 512*512
     * 640*360
     * 640*480
     * 640*640
     * 680*512
     * 512*680
     * 768*768
     * 720*1280
     * 1280*720
     * 1024*1024
     * **************************/
    LLM * llm = LLMFactory::imageGeneration(512,512,llmConfig);

    char input[1024];
    printf(GREEN "请输入图片主题,如需退出请输入q: \n" RESET);
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
        printf(YELLOW "图片生成中,请稍后... \n" RESET);
        LLMSyncOutput *result = llm->run(input);
        if (result->getContentType() == LLMResult::IMAGE) {

            /*******************获取交互结果**************************************/
            //解析获取的结果，示例展示所有结果获取，开发者可根据自身需要，选择获取。
            const char* content  = result->getContent();//获取返回结果
            int contentLen       = result->getContentLen();//获取返回结果长度
            SparkChain::LLMResult::ContentType type = result->getContentType();//获取返回结果类型,TEXT:返回结果为文本,IMAGE:返回结果为图片
            int errCode          = result->getErrCode();//获取结果状态,0:调用成功，非0:调用失败
            const char* errMsg   = result->getErrMsg();//获取失败时的错误信息
            const char* role     = result->getRole();//获取角色信息
            const char* sid      = result->getSid();//获取本次交互的sid
            int completionTokens = result->getCompletionTokens();//获取回答的Token大小
            int promptTokens     = result->getPromptTokens();//包含历史问题的总Tokens大小
            int totalTokens      = result->getTotalTokens();//promptTokens和completionTokens的和，也是本次交互计费的Tokens大小
            /*******************获取交互结果**************************************/

            if(errCode == 0){
                FILE * ptr = fopen("./syncpic.jpg", "w");  
                fwrite(content, contentLen, 1, ptr);
                fclose(ptr);
                printf(YELLOW "您的图片已生成，存放路径：./syncpic.jpg\n" RESET);
            }else{
                printf(RED "请求出错,错误码: %d,错误信息:%s\n\n" RESET, errCode, errMsg);
                break;
            }
        }
        printf(GREEN "请输入交互内容,如需退出请输入q: \n" RESET);    
    }
    // 运行结束，释放实例  
    if (llm != nullptr)
    {
        LLM::destroy(llm);
    }
    printf(GREEN "同步演示完成,请继续输入指令:\n0:同步生图\n1:异步生图\n2:退出\n" RESET);
}


void uninitSDK()
{
    // SDK逆初始化
    SparkChain::unInit();
}

int main(int argc, char const *argv[])
{
    int choice = 1; //演示模式选择，0：同步，1：异步。

    /* SDK初始化,初始化仅需全局初始化一次。*/
    int ret = initSDK();

    if (ret != 0)
    {
        printf(RED "\nSDK初始化失败!错误码:%d" RESET,ret);
        goto exit; //SDK初始化失败，退出
    }

    printf(GREEN "\n###############################################\n" RESET);
    printf(GREEN "##图片生成:用户输入文本,大模型根据文本生成相应的图片##\n" RESET);
    printf(GREEN "#################################################\n" RESET);
    printf(GREEN "演示示例选择:\n0:同步生图\n1:异步生图\n2:退出\n" RESET);

    while(1)
    {
        scanf("%d", &choice);
        if(choice == 1){
            printf(GREEN "异步生图演示\n" RESET);
            run_imageGeneration_Async();
        }else if(choice == 0){
            printf(GREEN "同步生图演示\n" RESET);
            run_imageGeneration_Sync(); 
        }else{
            break;
        }
    }

exit:
	printf(RED "已退出演示 ...\n" RESET);
	uninitSDK(); //退出    
}

