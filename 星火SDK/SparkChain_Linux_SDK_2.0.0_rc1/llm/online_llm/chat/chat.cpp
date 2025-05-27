/*************************
 * 星火大模型文本交互Demo (附带人物设定 - JSON模式)
 * create by qzy
 * 2025-5-17
 * Modified to include persona using raw JSON input and address previous issues.
 * Reduced verbosity of JSON logging.
 * ***********************/
#include "../../../include/sparkchain.h" // 假设这是正确的 SDK 头文件路径
#include <iostream>
#include <string>
#include <atomic>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Required for strlen, strcmp, fgets
#include <sstream>  // MODIFICATION: Added for std::ostringstream
#include <iomanip>  // MODIFICATION: Added for std::fixed and std::setprecision
#include <alsa/asoundlib.h> // Include ALSA header
#include "qtts.h"
#include "msp_cmn.h"
#include "msp_errors.h"

/* wav音频头部格式 */
typedef struct _wave_pcm_hdr
{
	char            riff[4];                // = "RIFF"
	int		size_8;                 // = FileSize - 8
	char            wave[4];                // = "WAVE"
	char            fmt[4];                 // = "fmt "
	int		fmt_size;		// = 下一个结构体的大小 : 16

	short int       format_tag;             // = PCM : 1
	short int       channels;               // = 通道数 : 1
	int		samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
	int		avg_bytes_per_sec;      // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
	short int       block_align;            // = 每采样点字节数 : wBitsPerSample / 8
	short int       bits_per_sample;        // = 量化比特数: 8 | 16

	char            data[4];                // = "data";
	int		data_size;              // = 纯数据长度 : FileSize - 44
} wave_pcm_hdr;

/* 文本合成 */
int text_to_speech(const char* src_text, const char* params)
{
	int          ret          = -1;
	const char*  sessionID    = NULL;
	unsigned int audio_len    = 0;
	int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

	// ALSA variables
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hw_params;
	int dir;
	int rc;
	int size;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE; // 16-bit little-endian PCM
	unsigned int rate = 16000; // Sample rate
	int channels = 1; // Channels

	if (NULL == src_text)
	{
		printf("params is error!\n");
		return ret;
	}

	/* Open PCM device for playback */
	rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0) {
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
		return rc;
	}

	/* Allocate a hardware parameters object */
	snd_pcm_hw_params_alloca(&hw_params);

	/* Fill it in with default values. */
	rc = snd_pcm_hw_params_any(handle, hw_params);
	if (rc < 0) {
		fprintf(stderr, "Failed to get hardware parameters: %s\n", snd_strerror(rc));
		snd_pcm_close(handle);
		return rc;
	}

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	rc = snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0) {
		fprintf(stderr, "Failed to set access type: %s\n", snd_strerror(rc));
		snd_pcm_close(handle);
		return rc;
	}

	/* Signed 16-bit little-endian format */
	rc = snd_pcm_hw_params_set_format(handle, hw_params, format);
	if (rc < 0) {
		fprintf(stderr, "Failed to set format: %s\n", snd_strerror(rc));
		snd_pcm_close(handle);
		return rc;
	}

	/* Single channel (mono) */
	rc = snd_pcm_hw_params_set_channels(handle, hw_params, channels);
	if (rc < 0) {
		fprintf(stderr, "Failed to set channels: %s\n", snd_strerror(rc));
		snd_pcm_close(handle);
		return rc;
	}

	/* 16000 bits/second sampling rate */
	rate = 16000; // Ensure rate is set before calling snd_pcm_hw_params_set_rate_near
	rc = snd_pcm_hw_params_set_rate_near(handle, hw_params, &rate, &dir);
	if (rc < 0) {
		fprintf(stderr, "Failed to set rate: %s\n", snd_strerror(rc));
		snd_pcm_close(handle);
		return rc;
	}

	/* Write the parameters to the PCM device */
	rc = snd_pcm_hw_params(handle, hw_params);
	if (rc < 0) {
		fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
		snd_pcm_close(handle);
		return rc;
	}

	/* Use a buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(hw_params, (snd_pcm_uframes_t*)&size, &dir);
	// Buffer size in bytes
	size = size * channels * snd_pcm_format_physical_width(format) / 8;


	/* 开始合成 */
	sessionID = QTTSSessionBegin(params, &ret);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionBegin failed, error code: %d.\n", ret);
		snd_pcm_close(handle);
		return ret;
	}
	ret = QTTSTextPut(sessionID, src_text, (unsigned int)strlen(src_text), NULL);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSTextPut failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "TextPutError");
		snd_pcm_close(handle);
		return ret;
	}
	printf("正在合成 ...\n");

	while (1)
	{
		/* 获取合成音频 */
		const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (MSP_SUCCESS != ret)
			break;
		if (NULL != data)
		{
			// Write data to ALSA device
			rc = snd_pcm_writei(handle, data, audio_len / (channels * snd_pcm_format_physical_width(format) / 8));
			if (rc < 0) {
				fprintf(stderr, "write to PCM device failed: %s\n", snd_strerror(rc));
				// Attempt to recover from error
				if (rc == -EPIPE) {
					/* EPIPE means underrun */
					fprintf(stderr, "ALSA underrun occurred\n");
					snd_pcm_prepare(handle);
				} else {
					break; // Exit loop on other errors
				}
			} else if (rc != audio_len / (channels * snd_pcm_format_physical_width(format) / 8)) {                fprintf(stderr, "short write, wrote %d frames instead of %u\n", rc, audio_len / (channels * snd_pcm_format_physical_width(format) / 8));
			}
		}
		if (MSP_TTS_FLAG_DATA_END == synth_status)
			break;
		printf(">");
		usleep(150*1000); //防止频繁占用CPU
	}
	printf("\n");

	// Drain the PCM device to ensure all buffered data is played
	snd_pcm_drain(handle);

	if (MSP_SUCCESS != ret)
	{
		printf("QTTSAudioGet failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "AudioGetError");
		snd_pcm_close(handle);
		return ret;
	}

	/* 合成完毕 */
	ret = QTTSSessionEnd(sessionID, "Normal");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionEnd failed, error code: %d.\n",ret);
	}

	snd_pcm_close(handle); // Close ALSA device

	return ret;
}


#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define RESET "\033[0m"

// TTS session parameters (should match the ones used in tts_online_sample)
const char* TTS_SESSION_BEGIN_PARAMS = "voice_name = aisjinger, text_encoding = utf8, sample_rate = 16000, speed = 70, volume = 80, pitch = 50, rdn = 2";

using namespace SparkChain;
using namespace std;

// async status tag
static atomic_bool finish(false);

// MODIFICATION START: 定义人物设定和相关参数
const char* PERSONA_SYSTEM_CONTENT = "你是上海半醒科技有限公司开发的人形机器人，名字叫做“精灵”，为客户提供服务，语言要简洁明了，不啰嗦。";
const char* USER_ID_FOR_MODEL = "user_elf_robot_session_001"; // 为每个用户或会话设置唯一ID
const float TEMPERATURE_FOR_MODEL = 0.5f;
const int MAX_TOKENS_FOR_MODEL = 80;
const char* DOMAIN_FOR_MODEL = "generalv3.5"; // 根据需要选择模型版本，确保与JSON中一致
// MODIFICATION END: 定义人物设定和相关参数

/*************************************SDK初始化参数**********************************************************/
// 这些APPID, APIKEY, APISECRET 用于SDK初始化，APPID也会用在JSON的header中
const char* APPID     = "69a4ef4e";         //用户的APPID (请确保这是您正确的APPID)
const char* APIKEY    = "026e8878757790d1f0a537ef3985aa0f"; //用户的APIKey (请确保这是您正确的APIKey)
const char* APISECRET = "NDMxYzMzMTI3MzU4NDc5YTdmMDFkNWFm"; //用户的APISecret (请确保这是您正确的APISecret)
const char* WORKDIR   = "./";               //SDK工作目录，要求有读写权限
int logLevel     = 100;                     //日志等级
/*************************************SDK初始化参数**********************************************************/

//交互结果监听回调
class SparkCallbacks : public LLMCallbacks
{
private:
    std::string accumulated_text; // Member to accumulate text chunks
    // TTS session parameters (should match the ones used in tts_online_sample)
    const char* tts_session_begin_params = "voice_name = aisjinger, text_encoding = utf8, sample_rate = 16000, speed = 70, volume = 80, pitch = 50, rdn = 2";


public:
    SparkCallbacks() : accumulated_text("") {} // Constructor to initialize

    void onLLMResult(LLMResult *result, void *usrContext) override
    {
        if(result->getContentType() == LLMResult::TEXT){
            const char* content  = result->getContent();
            int status           = result->getStatus();            printf(YELLOW "%s" RESET, content);
            fflush(stdout); // Flush output immediately
            
            accumulated_text += content; // Accumulate text chunks
            
            if (status == 2) {
                printf(YELLOW "\n" RESET);
                finish = true;                // Call text_to_speech with the accumulated content
                if (!accumulated_text.empty()) {
                    fprintf(stderr, "DEBUG: About to call TTS with text (length=%zu): '%s'\n", 
                            accumulated_text.length(), accumulated_text.c_str());
                    
                    // 写入控制文件通知ASR暂停监听
                    FILE* pause_file = fopen("/home/bxi/M2_SDK/log/tts_speaking", "w");
                    if (pause_file != NULL) {
                        fprintf(pause_file, "pause\n");
                        fclose(pause_file);
                        fprintf(stderr, "DEBUG: TTS signaled ASR to pause via /home/bxi/M2_SDK/log/tts_speaking\n");
                    }
                    
                    // 等待更长时间确保ASR已完全暂停
                    usleep(100000); // 100ms
                    
                    // 进行语音合成
                    fprintf(stderr, "DEBUG: Calling text_to_speech function...\n");
                    int tts_result = text_to_speech(accumulated_text.c_str(), tts_session_begin_params);
                    fprintf(stderr, "DEBUG: text_to_speech returned: %d\n", tts_result);
                    
                    accumulated_text.clear(); // Clear accumulated text after synthesis
                    
                    // 语音合成结束后，删除控制文件，通知ASR可以继续监听
                    if (remove("/home/bxi/M2_SDK/log/tts_speaking") == 0) {
                        fprintf(stderr, "DEBUG: TTS signaled ASR to resume by removing /home/bxi/M2_SDK/log/tts_speaking\n");
                    }
                    
                    // 给ASR足够的时间恢复监听
                    usleep(100000); // 100ms
                } else {
                    fprintf(stderr, "DEBUG: accumulated_text is empty, skipping TTS\n");
                }
            }
        }
    }
    
    void onLLMEvent(LLMEvent *event, void *usrContext) override
    {
        // 必须实现这个纯虚函数，即使为空
    }
      void onLLMError(LLMError *error, void *usrContext) override
    {
        fprintf(stderr, RED "Spark LLM Error: code=%d, msg=%s, sid=%s\n" RESET, error->getErrCode(), error->getErrMsg(), error->getSid());
        finish = true;
        accumulated_text.clear(); // Clear on error
    }
};

/***
 * SDK初始化 (保持不变)
 * ***/
int initSDK()
{
    SparkChainConfig *config = SparkChainConfig::builder();
    config->appID(APPID)
        ->apiKey(APIKEY)
        ->apiSecret(APISECRET)
        ->workDir(WORKDIR)
        ->logLevel(logLevel);
    int ret = SparkChain::init(config);
    return ret;
}

// MODIFICATION START: Helper function to escape JSON strings
std::string escapeJsonString(const std::string& input) {
    std::ostringstream oss;
    for (char c : input) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}
// MODIFICATION END: Helper function to escape JSON strings


/***
 * 异步请求 - 修改为使用 arunWithJson 实现人物设定
 * ***/
void run_Generation_Async()
{
    LLMConfig *llmConfig = nullptr;
    LLM *asyncllm = nullptr;
    SparkCallbacks *cbs = nullptr;

    llmConfig = LLMConfig::builder();
    if (!llmConfig) {
        fprintf(stderr, RED "LLMConfig::builder() failed.\n" RESET);
        return;
    }

    // MODIFICATION START: LLMConfig 主要用于设置 domain
    llmConfig->domain(DOMAIN_FOR_MODEL); // 例如 "generalv3.5" 或 "4.0Ultra"
    fprintf(stderr, GREEN "[Config Info] LLMConfig domain set to: %s\n" RESET, DOMAIN_FOR_MODEL);
    // 其他参数如 temperature, max_tokens, uid 将直接写入 JSON
    // MODIFICATION END: LLMConfig 主要用于设置 domain

    // MODIFICATION START: Memory object might not be directly used by arunWithJson for history.
    // For now, we pass nullptr for memory to LLMFactory, as history for arunWithJson needs manual JSON construction.
    // If your SDK version requires a Memory object even for arunWithJson, you can reinstate:
    // Memory* window_memory = Memory::WindowMemory(0); // Or some other appropriate value.
    // asyncllm = LLMFactory::textGeneration(llmConfig, window_memory);
    asyncllm = LLMFactory::textGeneration(llmConfig, nullptr); // Pass nullptr for memory for now
    // MODIFICATION END

    if (asyncllm == nullptr)
    {
        fprintf(stderr, RED "LLM instance creation failed, exiting.\n" RESET);
        if (llmConfig != nullptr) {
            // delete llmConfig; // VERIFY LLMConfig lifecycle with SDK docs
            llmConfig = nullptr;
        }
        return;
    }

    cbs = new SparkCallbacks();
    if (!cbs) {
        fprintf(stderr, RED "Failed to create SparkCallbacks instance.\n" RESET);
        LLM::destroy(asyncllm);
        if (llmConfig != nullptr) { /* delete llmConfig; */ } // VERIFY
        return;
    }
    asyncllm->registerLLMCallbacks(cbs);    char inputBuffer[4096];
    const char* input_file_path = "/home/bxi/M2_SDK/log/asr_output.txt";
    long last_file_size = -1;

    fprintf(stderr, GREEN "Spark LLM ready (using JSON mode with persona '精灵'). Waiting for input from %s...\n" RESET, input_file_path);

    const char* prefix = "IAT Partial Result: [ ";
    size_t prefix_len = strlen(prefix);

    while(true) {
        FILE* infile = fopen(input_file_path, "r");
        if (infile == NULL) {
            sleep(1);
            continue;
        }

        fseek(infile, 0, SEEK_END);
        long current_file_size = ftell(infile);

        if (current_file_size > 0 && current_file_size != last_file_size) {
            fseek(infile, 0, SEEK_SET);
            if (fgets(inputBuffer, sizeof(inputBuffer), infile) != NULL) {
                inputBuffer[strcspn(inputBuffer, "\n")] = 0; 

                if (strlen(inputBuffer) > 0) {
                    if (strncmp(inputBuffer, prefix, prefix_len) == 0) {
                        char* closing_bracket = strrchr(inputBuffer, ']');
                        if (closing_bracket != NULL && closing_bracket > inputBuffer + prefix_len) {
                            size_t transcribed_len = closing_bracket - (inputBuffer + prefix_len);
                            string transcribed_text_str(inputBuffer + prefix_len, transcribed_len);

                            size_t end_pos = transcribed_text_str.find_last_not_of(" \t\n\r\f\v");
                            if (string::npos != end_pos) {
                                transcribed_text_str = transcribed_text_str.substr(0, end_pos + 1);
                            }

                            if (!transcribed_text_str.empty()) {
                                fprintf(stderr, YELLOW "Read from file: [%s]\n" RESET, inputBuffer);
                                fprintf(stderr, YELLOW "Extracted transcribed text: [%s]\n" RESET, transcribed_text_str.c_str());
                                
                                // MODIFICATION START: Construct JSON payload
                                std::ostringstream json_payload_stream;
                                json_payload_stream << "{";
                                json_payload_stream << "\"header\": {";
                                json_payload_stream << "\"app_id\": \"" << APPID << "\",";
                                json_payload_stream << "\"uid\": \"" << USER_ID_FOR_MODEL << "\"";
                                json_payload_stream << "},";
                                json_payload_stream << "\"parameter\": {";
                                json_payload_stream << "\"chat\": {";
                                json_payload_stream << "\"domain\": \"" << DOMAIN_FOR_MODEL << "\",";
                                json_payload_stream << "\"temperature\": " << std::fixed << std::setprecision(1) << TEMPERATURE_FOR_MODEL << ",";
                                json_payload_stream << "\"max_tokens\": " << MAX_TOKENS_FOR_MODEL;
                                // Add other parameters like top_k if needed
                                json_payload_stream << "}";
                                json_payload_stream << "},";
                                json_payload_stream << "\"payload\": {";
                                json_payload_stream << "\"message\": {";
                                json_payload_stream << "\"text\": [";
                                // 1. System message for persona
                                json_payload_stream << "{\"role\": \"system\", \"content\": \"" << escapeJsonString(PERSONA_SYSTEM_CONTENT) << "\"},";
                                // 2. Current user message
                                json_payload_stream << "{\"role\": \"user\", \"content\": \"" << escapeJsonString(transcribed_text_str) << "\"}";
                                // If implementing history, loop through history messages here
                                json_payload_stream << "]";
                                json_payload_stream << "}";
                                // "functions" part can be added here if needed, as per doc 13.1
                                json_payload_stream << "}";
                                json_payload_stream << "}";
                                std::string json_payload = json_payload_stream.str();
                                
                                // MODIFICATION: Commented out the line that prints the full JSON payload
                                // fprintf(stderr, YELLOW "Sending JSON to Spark: %s\n" RESET, json_payload.c_str());
                                fprintf(stderr, YELLOW "Sending request to Spark LLM...\n" RESET); // Replaced with a simpler message
                                // MODIFICATION END: Construct JSON payload

                                finish = false;
                                // MODIFICATION START: Call arunWithJson
                                int ret_arun = asyncllm->arunWithJson(json_payload.c_str(), nullptr);
                                // MODIFICATION END: Call arunWithJson
                                if (ret_arun != 0) {
                                    fprintf(stderr, RED "Spark async request (arunWithJson) failed, error code: %d\n" RESET, ret_arun);
                                    finish = true; 
                                }

                                int times = 0;
                                while (!finish) {
                                    sleep(1);
                                    if (times++ > 30) { 
                                        fprintf(stderr, RED "Spark response timed out for input: [%s]\n" RESET, transcribed_text_str.c_str());
                                        break; 
                                    }
                                }
                            } else {
                                fprintf(stderr, YELLOW "Extracted empty transcribed text, skipping.\n" RESET);
                            }
                        } else {
                            fprintf(stderr, YELLOW "Ignoring line from file: Malformed (missing closing bracket or content): [%s]\n" RESET, inputBuffer);
                        }
                    } else {
                        fprintf(stderr, YELLOW "Ignoring line from file: Does not match prefix: [%s]\n" RESET, inputBuffer);
                    }
                }
            }
            last_file_size = current_file_size;
        }
        fclose(infile);
        sleep(1); 
    }

    fprintf(stderr, GREEN "Exiting Spark LLM interaction loop.\n" RESET);

    if (asyncllm != nullptr) { LLM::destroy(asyncllm); asyncllm = nullptr; }
    if (cbs != nullptr) { delete cbs; cbs = nullptr; }
    if (llmConfig != nullptr) {
        // delete llmConfig; // VERIFY LLMConfig lifecycle with SDK docs
        llmConfig = nullptr;
    }
    // If window_memory was used:
    // if (window_memory != nullptr) { /* Check SDK for Memory::destroy or similar */ window_memory = nullptr; }
}

void uninitSDK()
{
    SparkChain::unInit();
}

int main(int argc, char const *argv[])
{
    setvbuf(stdin, NULL, _IOLBF, 0);
    int ret = initSDK();
    if (ret != 0) {
        fprintf(stderr, RED "\nSDK initialization failed! Error code:%d\n" RESET, ret);
        return 1;
    }

    // TTS login parameters (should match the ones used in tts_online_sample)
    const char* tts_login_params = "appid = 69a4ef4e, work_dir = ."; // Use the same appid as LLM and specify work_dir

    // TTS User login
    ret = MSPLogin(NULL, NULL, tts_login_params); // First param username, second password, third login params
    if (MSP_SUCCESS != ret)
    {
        printf("MSPLogin failed for TTS, error code: %d.\n", ret);
        MSPLogout(); // Ensure logout if login fails
        return ret;
    }


    run_Generation_Async();

    MSPLogout(); // Logout from TTS
    uninitSDK(); // Uninitialize SparkChain SDK
    fprintf(stderr, RED "Spark LLM program finished.\n" RESET);
    return 0;
}
