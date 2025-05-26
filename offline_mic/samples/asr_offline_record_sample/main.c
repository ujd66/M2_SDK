/*************************************************************************************
@company: WHEELTEC (Dongguan) Co., Ltd
@product: 4/6mic
@filename: main.c
@brief:
@version:       date:       author:            comments:
@v1.0           22-4-16      hj,choi                main
*************************************************************************************/

#include "com_test.h" // Required for deal_with function declaration
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h> // Required for pthread_t
#include <time.h> // Required for time()

// IAT SDK Headers
#include "speech_recognizer.h" // 包含speech_rec和speech_rec_notifier结构体定义
#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include "linuxrec.h" // Required for open_port and set_opt

// #include "record.h" // SR_MIC模式下，SDK自行处理录音，暂时不需要此文件

#define FRAME_LEN	640
#define	BUFFER_SIZE	4096


extern int angle_int;
extern int if_awake;
// Provide definitions for init_success and record_finish
int init_success = 1; // Initialized to 1 as it was in the original M2 SDK main.c (implicitly)
int record_finish = 0;  // Initialized to 0 as it was in the original M2 SDK main.c (implicitly)

int once_awake;
int wake_switch = 1;
int in_conversation_mode = 0; // Added for multi-turn conversation
volatile int pause_wakeup_read = 0; // Added to control wakeup thread read

// Globals and Callbacks from iat_online_record_sample.c
static char *g_result = NULL;
static unsigned int g_buffersize = BUFFER_SIZE;

// Modified show_iat_result_callback to return the formatted string instead of printing
static char* show_iat_result_callback(char *string, char is_over)
{
	// This function now formats the partial result string and returns it.
	// It does NOT print to stdout or stderr.
	if(!is_over) {
		// Allocate buffer for formatted string. Max length of string + "IAT Partial Result: [ ]\r" + null terminator
		size_t formatted_len = strlen(string) + strlen("\rIAT Partial Result: [ ]") + 1;
		char* formatted_string = (char*)malloc(formatted_len);
		if (formatted_string) {
			snprintf(formatted_string, formatted_len, "\rIAT Partial Result: [ %s ]", string);
			return formatted_string;
		} else {
			fprintf(stderr, "Formatted string alloc failed\n");
			return NULL;
		}
	}
	return NULL; // Return NULL for final result or if not needed
}

void on_result(const char *result, char is_last)
{
	// 首先检查TTS是否正在播放，如果是则忽略此结果
    FILE* tts_check = fopen("/tmp/tts_speaking", "r");
    if (tts_check != NULL) {
        fprintf(stderr, "DEBUG: Ignoring ASR result because TTS is currently speaking\n");
        fclose(tts_check);
        return; // 直接返回，不处理这个识别结果
    }
	
	if (result) {
		size_t left = g_buffersize - 1 - strlen(g_result);
		size_t current_result_len = strlen(result);
		if (left < current_result_len) {
			g_result = (char*)realloc(g_result, g_buffersize + BUFFER_SIZE);
			if (g_result)
				g_buffersize += BUFFER_SIZE;
			else {
				fprintf(stderr, "mem alloc failed\n");
				return;
			}
		}
		strncat(g_result, result, current_result_len); // Corrected strncat usage

		if (is_last) {
            // Output final result to stdout for terminal/pipe
            printf("%s\n", g_result);
            fflush(stdout);

            // Also write final result to a temporary file for chat_demo to read
            FILE* outfile = fopen("/tmp/asr_output.txt", "w"); // Open in write mode to overwrite
            if (outfile != NULL) {
                fprintf(outfile, "IAT Partial Result: [ %s ]\n", g_result);
                fclose(outfile);
                fprintf(stderr, "DEBUG: Final result written to /tmp/asr_output.txt: IAT Partial Result: [ %s ]\n", g_result); // Log to stderr
            } else {
                fprintf(stderr, "ERROR: Failed to open /tmp/asr_output.txt for writing.\n");
            }


            // Check if the final result contains the exit keyword
            if (strstr(g_result, "再见") != NULL) {
                fprintf(stderr, "\nIAT: Detected exit keyword. Ending conversation.\n"); // Print to stderr (goes to log file)
                in_conversation_mode = 0; // Set flag to exit conversation mode
            }

		} else {
			// Get the formatted partial result string
			char* partial_result_str = show_iat_result_callback(g_result, is_last);
			if (partial_result_str) {
				// Print the formatted partial result to stderr (goes to log file)
				fprintf(stderr, "%s\n", partial_result_str);
				fflush(stderr); // Use stderr for partial results
				free(partial_result_str); // Free the allocated string
			}
		}
	}
}

void on_speech_begin()
{
	if (g_result)
	{
		free(g_result);
	}
	g_result = (char*)malloc(BUFFER_SIZE);
	g_buffersize = BUFFER_SIZE;
	memset(g_result, 0, g_buffersize);

	fprintf(stderr, "IAT: Start Listening...\n"); // Print to stderr
}

void on_speech_end(int reason)
{
	if (reason == END_REASON_VAD_DETECT)
		fprintf(stderr, "\nIAT: Speaking done (VAD)\n"); // Print to stderr
	else
		fprintf(stderr, "\nIAT: Recognizer error %d\n", reason); // Print to stderr
}
// End of Globals and Callbacks

void *com_wakeup(void* args)
{
    int fd=1, read_num = 0;
    unsigned char buffer[1];
    memset(buffer, 0, 1);
    char* uartname="/dev/wheeltec_mic";
    if((fd=open_port(uartname))<0)
    {
         fprintf(stderr, "open %s is failed\n",uartname); // Print to stderr
         return 0;
    }
    else{
            set_opt(fd, 115200, 8, 'N', 1);
            //fprintf(stderr, "set_opt fd=%d\n",fd); // Print to stderr

            while(1){
                    usleep(1000);
                    
                    // 检查TTS是否正在播放
                    FILE* tts_file = fopen("/tmp/tts_speaking", "r");
                    if (tts_file != NULL) {
                        pause_wakeup_read = 1;
                        static int pause_counter = 0;
                        if (pause_counter++ % 100 == 0) { // 减少日志输出频率
                            fprintf(stderr, "DEBUG: ASR paused due to TTS speaking\n");
                        }
                        fclose(tts_file);
                    } else {
                        if (pause_wakeup_read) {
                            // 文件不存在但状态还是暂停，需要延迟一段时间再恢复
                            static int resume_delay = 0;
                            if (resume_delay++ > 50) { // 延迟约50ms再恢复
                                pause_wakeup_read = 0;
                                resume_delay = 0;
                                fprintf(stderr, "DEBUG: ASR resumed after TTS finished with delay\n");
                            }
                        }
                    }
                    
                    // Check the flag before attempting to read
                    if (pause_wakeup_read) {
                        usleep(10000); // Sleep longer when paused to reduce CPU usage
                        continue; // Skip reading and check flag again
                    }

                    memset(buffer, 0, 1);
                    read_num = read(fd, buffer, 1);
                    //fprintf(stderr, "read_num=%d\n",read_num); // Print to stderr
                    if(read_num>0){
						//fprintf(stderr, "%x\n", buffer[0]); // Print to stderr
						// deal_with should set if_awake = 1 when wakeup is detected
						deal_with(buffer[0]);
                    }
                    //else{
                    //    fprintf(stderr, "read error\n"); // Print to stderr
                    //}
            }
          fd=close(fd); // 这行代码实际上永远不会执行到，因为上面是while(1)无限循环
    }
    return 0;
}


int main()
{
    // Simple test output to stdout
    printf("ASR程序已启动\n");
    fflush(stdout);

    pthread_t pid1;
    
    // 启动时强制清理可能存在的TTS控制文件
    // 多次尝试确保删除成功
    for(int i=0; i<3; i++) {
        if (remove("/tmp/tts_speaking") == 0) {
            fprintf(stderr, "INFO: Removed existing /tmp/tts_speaking file at startup\n");
            break;
        }
        // 如果文件不存在或者其他原因导致删除失败，则尝试创建并立即删除
        FILE* f = fopen("/tmp/tts_speaking", "w");
        if (f != NULL) {
            fclose(f);
            remove("/tmp/tts_speaking");
        }
        usleep(100000); // 等待100ms再尝试
    }
    
    // 确保控制变量初始状态正确
    pause_wakeup_read = 0;

    // Open log file and redirect stderr
    const char* home_dir = getenv("HOME");
    if (home_dir == NULL) {
        fprintf(stderr, "Failed to get home directory.\n");
        return -1;
    }
    char log_file_path[256];
    snprintf(log_file_path, sizeof(log_file_path), "%s/M2_SDK/asr_log.txt", home_dir);

    FILE* log_file = fopen(log_file_path, "w");
    if (log_file == NULL) {
        fprintf(stderr, "CRITICAL_ERROR: Failed to open log file at %s. Stderr will not be redirected.\n", log_file_path);
    } else {
        if (dup2(fileno(log_file), fileno(stderr)) == -1) {
            fprintf(stderr, "CRITICAL_ERROR: Failed to redirect stderr. Stderr will not be redirected.\n");
        } else {
             fprintf(stderr, "Log output (stderr) redirected to %s\n", log_file_path);
        }
    }

    printf("ASR程序已启动 (stdout test)\n");
    fflush(stdout);
    fprintf(stderr, "ASR程序已启动 (stderr test)\n");
    fflush(stderr);

    const char* login_params = "appid = 69a4ef4e, work_dir = .";
    int ret_msp = MSPLogin(NULL, NULL, login_params);
    if (MSP_SUCCESS != ret_msp) {
        fprintf(stderr, "MSPLogin failed, Error code %d.\n", ret_msp);
        return -1;
    }
    fprintf(stderr, "MSPLogin successful.\n");

    init_success = 1;

    if(pthread_create(&pid1, NULL, com_wakeup, NULL))
    {
        fprintf(stderr, "Failed to create com_wakeup thread.\n"); // 添加错误日志
        MSPLogout();
        return -1;
    }


    // Main loop to wait for wakeup and manage multi-turn conversation
    while(1){
        time_t last_speech_time; // Declare variable for tracking last speech time

        if (!if_awake)
        {
            printf(">>>>>待唤醒，请用唤醒词进行唤醒！\n");
            fflush(stdout);
        }
        while(!if_awake)
        {
            sleep(1);
        }

        printf("\n>>>>>唤醒成功！角度: [%d]\n", angle_int);
        fflush(stdout);

        // 新增功能：输出固定的 ASR 结果
        const char* predefined_asr_output = "你好精灵，请介绍一下你自己";
        printf("%s\n", predefined_asr_output);
        fflush(stdout);

        FILE* outfile = fopen("/tmp/asr_output.txt", "w");
        if (outfile != NULL) {
            fprintf(outfile, "IAT Partial Result: [ %s ]\n", predefined_asr_output);
            fclose(outfile);
            fprintf(stderr, "DEBUG: Predefined ASR result written to /tmp/asr_output.txt: IAT Partial Result: [ %s ]\n", predefined_asr_output);
        } else {
            fprintf(stderr, "ERROR: Failed to open /tmp/asr_output.txt for writing predefined ASR result.\n");
        }
        // 新增功能结束

        in_conversation_mode = 1;
        last_speech_time = time(NULL); // Initialize last speech time

        // Multi-turn conversation loop
        while(in_conversation_mode) {
            // Check for 100 seconds of inactivity
            if (time(NULL) - last_speech_time > 100) {
                in_conversation_mode = 0; // Exit conversation mode
                fprintf(stderr, "DEBUG: 100 seconds of inactivity detected. Ending conversation.\n");
                break; // Exit the inner while loop
            }

            const char* session_begin_params =
                "sub = iat, domain = iat, language = zh_cn, "
                "accent = mandarin, sample_rate = 16000, "
                "result_type = plain, result_encoding = utf8, "
                "vad_eos = 3000, "
                "vad_bos = 3000, "
                "ptt = 1";

            struct speech_rec iat_session;
            struct speech_rec_notifier notifier_callbacks = { on_result, on_speech_begin, on_speech_end };
            int errcode_sr;

            // 检查TTS是否正在播放，如果正在播放则等待
            FILE* tts_file = NULL;
            int tts_wait_counter = 0;
            while ((tts_file = fopen("/tmp/tts_speaking", "r")) != NULL) {
                fprintf(stderr, "DEBUG: Main loop waiting for TTS to finish...\n");
                fclose(tts_file);
                sleep(1);
                if (++tts_wait_counter > 30) { // 最多等待30秒
                    fprintf(stderr, "WARNING: Timeout waiting for TTS to finish\n");
                    break;
                }
            }
            
            // 即使文件已删除，也需要额外等待一段时间，确保TTS播放完全结束
            // 并且ASR状态完全恢复
            fprintf(stderr, "DEBUG: TTS finished, waiting additional time for ASR stabilization...\n");
            usleep(100000); // 等待额外的100ms

            printf(">>>>>已开启在线实时语音听写！请说话...\n");
            fflush(stdout);
            fprintf(stderr, "DEBUG: Calling sr_init with params: %s\n", session_begin_params);

            errcode_sr = sr_init(&iat_session, session_begin_params, SR_MIC, &notifier_callbacks);
            if (errcode_sr != MSP_SUCCESS) {
                fprintf(stderr, "ERROR: sr_init failed, errorcode: %d\n", errcode_sr);
                in_conversation_mode = 0; // Exit conversation mode on error
                break; 
            }
            fprintf(stderr, "DEBUG: sr_init successful.\n");

            fprintf(stderr, "DEBUG: Calling sr_start_listening.\n");
            errcode_sr = sr_start_listening(&iat_session);
            if (errcode_sr != MSP_SUCCESS) {
                fprintf(stderr, "ERROR: sr_start_listening failed, errorcode: %d\n", errcode_sr);
                sr_uninit(&iat_session);
                in_conversation_mode = 0; // Exit conversation mode on error
                break;
            }
            fprintf(stderr, "DEBUG: sr_start_listening successful.\n");


            fprintf(stderr, "Listening for up to 10 seconds...\n");
            sleep(10); // Wait for the SDK to process audio and trigger callbacks

            fprintf(stderr, "DEBUG: Calling sr_stop_listening.\n");
            errcode_sr = sr_stop_listening(&iat_session);
            if (errcode_sr != MSP_SUCCESS) {
                fprintf(stderr, "ERROR: sr_stop_listening failed, errorcode: %d \n", errcode_sr);
            }
            fprintf(stderr, "DEBUG: sr_stop_listening finished.\n");

            fprintf(stderr, "DEBUG: Calling sr_uninit.\n");
            sr_uninit(&iat_session);
            fprintf(stderr, "DEBUG: IAT session uninitialized.\n");

            last_speech_time = time(NULL); // Update last speech time after session ends

            if (in_conversation_mode) {
                fprintf(stderr, "Starting next turn in 1 second...\n");
                sleep(1);
            }

        } // End of while(in_conversation_mode) - multi-turn loop

        // Exited conversation mode
        fprintf(stderr, "Exited conversation mode. Returning to idle.\n");

        // 清理 /tmp/asr_output.txt 文件
        // Clean up the /tmp/asr_output.txt file
        if (remove("/tmp/asr_output.txt") == 0) {
            fprintf(stderr, "DEBUG: Successfully deleted /tmp/asr_output.txt after conversation.\n");
        } else {
            // 如果删除失败，可能是因为文件不存在（这通常不是问题），
            // 或者因为权限等其他问题。
            // If remove fails, it could be because the file doesn't exist (which is often fine),
            // or due to permissions or other issues.
            // perror("DEBUG: Error deleting /tmp/asr_output.txt"); // 这会打印详细的系统错误信息
            fprintf(stderr, "DEBUG: Attempted to delete /tmp/asr_output.txt. File might not have existed or another error occurred.\n");
        }

        if_awake = 0; // Reset awake flag to wait for the next wakeup

    } // End of while(1) - main loop

    // 确保删除TTS控制文件
    remove("/tmp/tts_speaking");
    
    MSPLogout(); // Logout from MSC
    return 0;
}
