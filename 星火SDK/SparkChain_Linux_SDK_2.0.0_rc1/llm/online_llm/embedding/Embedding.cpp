/*************************
 * 文本向量化Demo
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
#include "../../../include/sc_embedding.h"//Embedding需要用的头文件

#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define RESET "\033[0m"

using namespace SparkChain;
using namespace std;

/*************************************SDK初始化参数**********************************************************/
char * APPID     = "APPID";                        //用户的APPID
char * APIKEY    = "APIKEY";                       //用户的APIKey
char * APISECRET = "APISECRET";                    //用户的APISecret
char * WORKDIR   = "./";                                    //SDK工作目录，要求有读写权限
int logLevel     = 100;                                     //日志等级。0：VERBOSE(日志最全)，1：DEBUG，2：INFO，3：WARN，4：ERROR，5：FATAL，100：OFF(关闭日志)
/*************************************SDK初始化参数**********************************************************/

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


void run_embeddingP(){
    /**************
     * 向量化，入参说明：
     * input:用户输入的文本
     * domain:query:用户问题向量化 para:文档向量化
     * ****************/
    EmbeddingOutput output = Embedding::embedding("这段话的内容变成向量化是什么样的？","para");
    int ret = output.getErrCode();
    if(ret == 0){
        std::vector<float> result = output.getResultVector();
        /***************结果存文件*******************/
        // 创建一个输出文件流对象
        std::ofstream outFile("embeddingP.txt");
        // 检查文件是否成功打开
        if (!outFile.is_open()) {
            printf(RED "无法打开embeddingP.txt文件,请教检查./下是否有读写权限。\n"RESET);
        }
        // 将vector中的数据写入文件
        for (const float &value : result) {
            outFile << value << std::endl;
        }
        // 关闭文件
        outFile.close();
        printf(YELLOW "知识原文向量化结束，向量文件存放在:./embeddingP.txt\n" RESET);
        printf(GREEN "知识原文向量化演示完成,请继续输入指令:\n0:知识原文向量化\n1:用户问题向量化\n2:退出\n" RESET);
        /***************结果存文件*******************/
    }else{
        printf(RED "EmbeddingP转换失败,错误码：%d\n" RESET ,ret);
    }
}

void run_embeddingQ(){
    /**************
     * 向量化，入参说明：
     * input:用户输入的文本
     * domain:query:用户问题向量化 para:文档向量化
     * ****************/
    EmbeddingOutput output = Embedding::embedding("这段话的内容变成向量化是什么样的？","query");
    int ret = output.getErrCode();
    if(ret == 0){
        std::vector<float> result = output.getResultVector();
        /***************结果存文件*******************/
        // 创建一个输出文件流对象
        std::ofstream outFile("embeddingQ.txt");
        // 检查文件是否成功打开
        if (!outFile.is_open()) {
            printf(RED"无法打开embeddingQ.txt文件,请教检查./下是否有读写权限。\n" RESET);
        }
        // 将vector中的数据写入文件
        for (const float &value : result) {
            outFile << value << std::endl;
        }
        // 关闭文件
        outFile.close();
        printf(YELLOW "用户问题向量化结束，向量文件存放在:./embeddingQ.txt\n" RESET);
        printf(GREEN "用户问题向量化演示完成,请继续输入指令:\n0:知识原文向量化\n1:用户问题向量化\n2:退出\n" RESET);
        /***************结果存文件*******************/
    }else{
        printf(RED"EmbeddingQ转换失败,错误码：%d\n" RESET ,ret);
    }
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
    printf(GREEN "##Embedding:把用户的输入进行向量化##\n" RESET);
    printf(GREEN "#################################################\n" RESET);
    printf(GREEN "演示示例选择:\n0:知识原文向量化\n1:用户问题向量化\n2:退出\n" RESET);

    while(1)
    {
        scanf("%d", &choice);
        if(choice == 1){
            printf(GREEN "用户问题向量化演示\n" RESET);
            run_embeddingQ();
        }else if(choice == 0){
            printf(GREEN "知识原文向量化演示\n" RESET);
            run_embeddingP(); 
        }else{
            break;
        }
    }

exit:
	printf(RED "已退出演示 ...\n" RESET);
	uninitSDK(); //退出    
}
