/*
* 语音合成（Text To Speech，TTS）技术能够自动将任意文字实时转换为连续的
* 自然语音，是一种能够在任何时间、任何地点，向任何人提供语音信息服务的
* 高效便捷手段，非常符合信息时代海量数据、动态更新和个性化查询的需求。
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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

/* 默认wav音频头部数据 */
wave_pcm_hdr default_wav_hdr = 
{
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	16000,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};
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
	unsigned int rate = 16000; // Sample rate from default_wav_hdr
	int channels = 1; // Channels from default_wav_hdr

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
			} else if (rc != audio_len / (channels * snd_pcm_format_physical_width(format) / 8)) {
				fprintf(stderr, "short write, wrote %d frames instead of %lu\n", rc, audio_len / (channels * snd_pcm_format_physical_width(format) / 8));
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

int main(int argc, char* argv[])
{
	int         ret                  = MSP_SUCCESS;
	const char* login_params         = "appid = 69a4ef4e, work_dir = .";//登录参数,appid与msc库绑定,请勿随意改动
	/*
	* rdn:           合成音频数字发音方式
	* volume:        合成音频的音量
	* pitch:         合成音频的音调
	* speed:         合成音频对应的语速
	* voice_name:    合成发音人
	* sample_rate:   合成音频采样率
	* text_encoding: 合成文本编码格式
	*
	*/
	const char* session_begin_params = "voice_name = aisjiuxu, text_encoding = utf8, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2";
	// const char* filename             = "tts_sample.wav"; //合成的语音文件名称 // Removed file writing
	const char* text                 = "我爱雪豹"; //合成文本

	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params);//第一个参数是用户名，第二个参数是密码，第三个参数是登录参数，用户名和密码可在http://www.xfyun.cn注册获取
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed, error code: %d.\n", ret);
		goto exit ;//登录失败，退出登录
	}
	printf("\n###########################################################################\n");
	printf("## 语音合成（Text To Speech，TTS）技术能够自动将任意文字实时转换为连续的 ##\n");
	printf("## 自然语音，是一种能够在任何时间、任何地点，向任何人提供语音信息服务的  ##\n");
	printf("## 高效便捷手段，非常符合信息时代海量数据、动态更新和个性化查询的需求。  ##\n");
	printf("###########################################################################\n\n");
	/* 文本合成 */
	printf("开始合成 ...\n");
	// ret = text_to_speech(text, filename, session_begin_params); // Modified function call
	ret = text_to_speech(text, session_begin_params);
	if (MSP_SUCCESS != ret)
	{
		printf("text_to_speech failed, error code: %d.\n", ret);
	}
	printf("合成完毕\n");

exit:
	printf("按任意键退出 ...\n");
	getchar();
	MSPLogout(); //退出登录

	return 0;
}
