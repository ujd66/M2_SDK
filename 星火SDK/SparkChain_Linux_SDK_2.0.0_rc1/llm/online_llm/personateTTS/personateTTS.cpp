/*************************
 * 超拟人合成Demo
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
#include "../../../include/sc_tts.h" //超拟人需要用的头文件

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
char* audioPath = "./audio.pcm";

/*************************************SDK初始化参数**********************************************************/
char * APPID     = "APPID";                        //用户的APPID
char * APIKEY    = "APIKEY";                       //用户的APIKey
char * APISECRET = "APISECRET";                    //用户的APISecret
char * WORKDIR   = "./";                                    //SDK工作目录，要求有读写权限
int logLevel     = 100;                                     //日志等级。0：VERBOSE(日志最全)，1：DEBUG，2：INFO，3：WARN，4：ERROR，5：FATAL，100：OFF(关闭日志)
/*************************************SDK初始化参数**********************************************************/

//超拟人合成监听回调
class PersonateCallbacks : public TTSCallbacks
{
    void onResult(TTSResult * result,void * usrTag) {

        const char * data = result->data();//音频数据
        int len           = result->len();//音频数据长度
        int status        = result->status();//数据状态
        string ced        = result->ced();//进度
        int seq           = result->seq();//数据序号
        string pybuf      = result->pybuf();//拼音结果
        string version    = result->version();//引擎版本号  
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

    void onError(TTSError * error, void * usrTag) {
        finish = true;
        int errCode   = error->code();//错误码
        string errMsg = error->errMsg();//错误信息
        string sid    = error->sid();//sid
        printf(RED "请求出错,错误码: %d,错误信息:%s,交互sid:%s\n" RESET, errCode, errMsg, sid);
    }    
};

void deleteCacheAudioFile(){
    // 检查文件是否存在
    if (access(audioPath, F_OK) != -1) {
        // 删除文件
        remove(audioPath);
        file = nullptr;
    } 
}


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




void run_personateTTS_flow(){
    deleteCacheAudioFile();
    char *input[] = {"技术支持跑的快，全靠大佬们带。","2024年9月26日，星期四，小明在这一天开启了一段充满惊喜的冒险之旅。","床前明月光，疑似地上霜。"};
    finish = false;
    /***********
     * 设置发音人：
     * x4_lingxiaoxuan_oral，聆⼩璇，⼥：中⽂
     * x4_lingfeiyi_oral，聆飞逸，男：中⽂
     * ************/
    PersonateTTS tts("x4_lingxiaoxuan_oral");
    tts.sampleRate(16000);//输出音频的采样率。16K:16000, 8K:8000, 24K:24000

    // tts.url("");//支持动态设置url
    PersonateCallbacks *cbs = new PersonateCallbacks();
    tts.registerCallbacks(cbs);//注册监听回调
    int length = sizeof(input) / sizeof(input[0]);
    for(int i = 0; i < length; i++){

        int status = 0;//输入文本状态，0:开始,1:中间,2:结束
        if(i == 0){
            status = 0;
        }else if(i == length-1){
            status = 2;
        }else{
            status = 1;
        }

        printf(GREEN "用户输入:%s \n" RESET,input[i]);
        printf(YELLOW "音频合成中,请稍后...\n" RESET);
        tts.arun(input[i],status);
    }
    while (!finish)
    { // 等待结果返回退出
        sleep(1);
    }
    printf(GREEN "流式合成演示完成,请继续输入指令:\n0:一次性合成:用户一次性输入所有文本进行合成\n1:流式合成:用户多次输入文本进行合成\n2:退出\n" RESET);
}

void run_personateTTS_once(){
    /***********
     * 设置发音人：
     * x4_lingxiaoxuan_oral，聆⼩璇，⼥：中⽂
     * x4_lingfeiyi_oral，聆飞逸，男：中⽂
     * ************/
    PersonateTTS tts("x4_lingxiaoxuan_oral");
    // PersonateTTS tts("x4_lingfeizhe_oral");

    tts.sampleRate(16000);//输出音频的采样率。16K:16000, 8K:8000, 24K:24000

    // tts.url("");//支持动态设置url
    PersonateCallbacks *cbs = new PersonateCallbacks();
    tts.registerCallbacks(cbs);//注册监听回调
    char input[1024];
    printf(GREEN "请输入合成文本,如需退出请输入q: \n" RESET);
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
        printf(YELLOW "音频合成中,请稍后... \n" RESET);
        finish = false;
        deleteCacheAudioFile();
        int ret = tts.arun(input);;
        if (ret != 0)
        {
            printf(RED "音频合成失败,错误码: %d\n" RESET, ret);
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
        printf(GREEN "请输入合成文本,如需退出请输入q: \n" RESET);
    }
    printf(GREEN "一次性合成演示完成,请继续输入指令:\n0:一次性合成:用户一次性输入所有文本进行合成\n1:流式合成:用户多次输入文本进行合成\n2:退出\n" RESET);
}


void uninitSDK()
{
    // 全局逆初始化
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
    printf(GREEN "##超拟人合成:用户输入文本,大模型合成音频##\n" RESET);
    printf(GREEN "#################################################\n" RESET);
    printf(GREEN "演示示例选择:\n0:一次性合成:用户一次性输入所有文本进行合成\n1:流式合成:用户多次输入文本进行合成\n2:退出\n" RESET);

    while(1)
    {
        scanf("%d", &choice);
        if(choice == 1){
            printf(GREEN "流式合成演示\n" RESET);
            run_personateTTS_flow();
        }else if(choice == 0){
            printf(GREEN "一次性合成演示\n" RESET);
            run_personateTTS_once(); 
        }else{
            break;
        }
    }

exit:
	printf(RED "已退出演示 ...\n" RESET);
	uninitSDK(); //退出    
}
