/*************************
 * 图片理解Demo
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
        //获取结果，示例展示所有结果获取，开发者可根据自身需要，选择获取。
        const char* content  = result->getContent();//获取返回结果
        int contentLen       = result->getContentLen();//获取返回结果长度
        SparkChain::LLMResult::ContentType type = result->getContentType();//获取返回结果类型,TEXT:返回结果为文本,IMAGE:返回结果为图片 
        int status           = result->getStatus();//返回结果状态
        const char* role     = result->getRole();//获取角色信息
        const char* sid      = result->getSid();//获取本次会话的sid
        int completionTokens = result->getCompletionTokens();//获取回答的Token大小
        int promptTokens     = result->getPromptTokens();//包含历史问题的总Tokens大小
        int totalTokens      = result->getTotalTokens();//promptTokens和completionTokens的和，也是本次交互计费的Tokens大小

        if(result->getContentType() == LLMResult::TEXT){
            printf(YELLOW "大模型回复: %s\n" RESET, content);
            if (status == 2)
            {
                finish = true;
            }  
            
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


void run_imageUnderstanding_Async(){
    char* imageData = nullptr;
    char* imagePath = "./游乐园.jpg";
    int imageLen = -1;
    char* input = "图片的内容是什么？";

    LLMConfig *llmConfig = LLMConfig::builder();
    llmConfig->maxToken(2048); //回答的tokens的最大长度，取值范围1-4096，默认2048。其他配置参数请参考集成文档


    /*******************
     * 设置历史上下文
     * WindowMemory：通过会话轮数控制上下文范围，即一次提问和一次回答为一轮会话交互。用户可指定会话关联几轮上下文。
     * TokenMemory： 通过Token总长度控制上下文范围，1 token 约等于1.5个中文汉字 或者 0.8个英文单词。用户可指定历史会话Token长度
     * ***********************/
    Memory* window_memory = Memory::WindowMemory(5);
    LLM * llm = LLMFactory::imageUnderstanding(llmConfig,window_memory);//以WindowsMemory创建LLM实例
    // Memory* token_memory = Memory::TokenMemory(500);
    // LLM * llm = LLMFactory::imageUnderstanding(llmConfig,token_memory);//以TokenMemory创建LLM实例

    finish = false;
    
    SparkCallbacks *cbs = new SparkCallbacks();
    llm->registerLLMCallbacks(cbs);// 注册监听回调
    /***********************读取图片 start******************************/
    printf(YELLOW "正在读取图片中...\n" RESET);
    // 打开图片文件
    FILE *file = NULL;
    file = fopen(imagePath, "rb");
    if (file == NULL) {
        printf(RED "无法打开图片文件： %s\n" RESET, imagePath);
        return;
    }
    // 获取图片文件大小
    fseek(file, 0, SEEK_END);
    imageLen = ftell(file);
    rewind(file);   
    // 分配内存空间存储图片数据
    imageData = (char *)malloc(imageLen);
    if (imageData == NULL) {
        printf(RED "内存分配失败\n" RESET);
        fclose(file);
        return;
    }
    // 读取图片数据到字符数组中
    int read_size = fread(imageData, 1, imageLen, file);
    if (read_size != imageLen) {
        printf(RED "读取图片数据失败\n" RESET);
        free(imageData);
        fclose(file);
        return;
    }
    fclose(file);
    printf(YELLOW "图片读取成功!图片大小:%d \n" RESET,imageLen);
    /***********************读取图片 end******************************/

    //图片理解支持带memory的交互，第一次交互时需传入图片，后续交互则可以不用传图片，SDK会自动把图片数据拼接后，一并上传
    //如果想开启新一轮会话，则需要先调用llm->clearHistory()方法清空memory
    // llm->clearHistory();
    
    printf(GREEN "用户提问:%s \n" RESET,input);
    printf(YELLOW "大模型理解中,请稍后... \n" RESET);
    llm->arun(input,imageData,imageLen);
    
    int times = 0;
    while (!finish)
    { // 等待结果返回退出
        sleep(1);
        if (times++ > 10) // 等待十秒如果没有最终结果返回退出
            break;
    }
    if (llm != nullptr)
    {
        LLM::destroy(llm);
    }
    if (cbs != nullptr)
        delete cbs;
    free(imageData);
    printf(GREEN "异步演示完成,请继续输入指令:\n0:同步交互\n1:异步交互\n2:退出\n" RESET);
}

void run_imageUnderstanding_Sync(){
    char* imageData = nullptr;
    char* imagePath = "./游乐园.jpg";
    int imageLen = -1;
    char* input = "图片的内容是什么?";

    LLMConfig *llmConfig = LLMConfig::builder();
    llmConfig->maxToken(2048); //回答的tokens的最大长度，取值范围1-4096，默认2048。其他配置参数请参考集成文档

    /*******************
     * 设置历史上下文
     * WindowMemory：通过会话轮数控制上下文范围，即一次提问和一次回答为一轮会话交互。用户可指定会话关联几轮上下文。
     * TokenMemory： 通过Token总长度控制上下文范围，1 token 约等于1.5个中文汉字 或者 0.8个英文单词。用户可指定历史会话Token长度
     * ***********************/
    Memory* window_memory = Memory::WindowMemory(5);
    LLM * llm = LLMFactory::imageUnderstanding(llmConfig,window_memory);//以WindowsMemory创建LLM实例
    // Memory* token_memory = Memory::TokenMemory(500);
    // LLM * llm = LLMFactory::imageUnderstanding(llmConfig,token_memory);//以TokenMemory创建LLM实例

    /***********************读取图片 start******************************/
    printf(YELLOW "正在读取图片中...\n" RESET);
    // 打开图片文件
    FILE *file = NULL;
    file = fopen(imagePath, "rb");
    if (file == NULL) {
        printf(RED "无法打开图片文件： %s\n" RESET, imagePath);
        return;
    }
    // 获取图片文件大小
    fseek(file, 0, SEEK_END);
    imageLen = ftell(file);
    rewind(file);   
    // 分配内存空间存储图片数据
    imageData = (char *)malloc(imageLen);
    if (imageData == NULL) {
        printf(RED "内存分配失败\n" RESET);
        fclose(file);
        return;
    }
    // 读取图片数据到字符数组中
    int read_size = fread(imageData, 1, imageLen, file);
    if (read_size != imageLen) {
        printf(RED "读取图片数据失败\n" RESET);
        free(imageData);
        fclose(file);
        return;
    }
    fclose(file);
    printf(YELLOW "图片读取成功!图片大小:%d \n" RESET,imageLen);
    /***********************读取图片 end******************************/

    //图片理解支持带memory的交互，第一次交互时需传入图片，后续交互则可以不用传图片，SDK会自动把图片数据拼接后，一并上传
    //如果想开启新一轮会话，则需要先调用llm->clearHistory()方法清空memory
    // llm->clearHistory();
    printf(GREEN "用户提问:%s \n" RESET,input);
    printf(YELLOW "大模型理解中,请稍后... \n" RESET);
    LLMSyncOutput *result = llm->run(input,imageData,imageLen);

    /*******************获取交互结果**************************************/
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

    if (errCode != 0){
        printf(RED "请求出错,错误码: %d,错误信息:%s\n\n" RESET, errCode, errMsg);
           
    }else{
        printf(YELLOW "大模型回复: %s\n" RESET, content);
    }

    free(imageData);

    // 运行结束，释放实例  
    if (llm != nullptr)
    {
        LLM::destroy(llm);
    }
    printf(GREEN "同步演示完成,请继续输入指令:\n0:同步交互\n1:异步交互\n2:退出\n" RESET);
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
    printf(GREEN "##图片理解:用户输入图片,大模型根据图片理解相应的内容##\n" RESET);
    printf(GREEN "#################################################\n" RESET);
    printf(GREEN "演示示例选择:\n0:同步交互\n1:异步交互\n2:退出\n" RESET);

    while(1)
    {
        scanf("%d", &choice);
        if(choice == 1){
            printf(GREEN "异步交互演示\n" RESET);
            run_imageUnderstanding_Async();
        }else if(choice == 0){
            printf(GREEN "同步交互演示\n" RESET);
            run_imageUnderstanding_Sync(); 
        }else{
            break;
        }
    }

exit:
	printf(RED "已退出演示 ...\n" RESET);
	uninitSDK(); //退出    
}
