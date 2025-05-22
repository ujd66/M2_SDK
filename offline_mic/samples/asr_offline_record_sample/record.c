/*************************************************************************************
@company: WHEELTEC (Dongguan) Co., Ltd
@product: 4/6mic
@filename: record.c
@brief:
@version:       date:       author:            comments:
@v1.0           22-4-16      hj                record
*************************************************************************************/

#include "record.h"

record_handle_t record = {0};
record_params_t param = 
    {
        2,      //format
        1,      //channel
        16000   //rate
    };

//通用格式选择
static snd_pcm_format_t get_format_from_params(record_params_t* params){
    if(params!=NULL){  
        switch (params->format) {  
        case 0:
            return SND_PCM_FORMAT_S8;
        case 1:
            return SND_PCM_FORMAT_U8;
        case 2:
            return SND_PCM_FORMAT_S16_LE;
        case 3:
            return SND_PCM_FORMAT_S16_BE;
        default:  return SND_PCM_FORMAT_S16_LE;
        }
    }
    return SND_PCM_FORMAT_S16_LE;
}

int Record_Params_Init(record_handle_t* pcm_handle,record_params_t* params)
{
    int er;
    unsigned int buffer_time, period_time; 

    if (pcm_handle == NULL)
    {
        return -1;
    }
    
    sleep(1);

    if (snd_pcm_open(&(pcm_handle->pcm), RECORD_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0) < 0) { 
        fprintf(stderr, "Error snd_pcm_open [ %s]\n", RECORD_DEVICE_NAME); 
        exit(1);
    } 

    //hwparams参数结构体
    snd_pcm_hw_params_t *hwparams;

    //在栈上分配 snd_pcm_hw_params_t 结构
    snd_pcm_hw_params_alloca(&hwparams);

    // 初始化完整的配置空间 hwparams
    if((er = snd_pcm_hw_params_any(pcm_handle->pcm,hwparams)) < 0)
    {
        printf("初始化参数结构失败 : [%s]\n",snd_strerror(er));
        goto Init_fail;
    }

    //数据存储方式设置(INTERLEAVED :数据交叉存放;NONINTERLEAVED :数据分声道存放)
    if ((er = snd_pcm_hw_params_set_access(pcm_handle->pcm,hwparams,SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        printf("访问类型设置失败 : [%s]\n",snd_strerror(er));
        goto Init_fail;
    }

    //获取格式设置
    pcm_handle->format = get_format_from_params(params);

    //设置pcm数据格式
    if ((er = snd_pcm_hw_params_set_format(pcm_handle->pcm,hwparams,pcm_handle->format)) < 0)
    {
        printf("设置pcm数据格式失败 : [%s]\n",snd_strerror(er));
        goto Init_fail;
    }

    //获取channel并设置

    if ((er = snd_pcm_hw_params_set_channels(pcm_handle->pcm,hwparams,params->channel)) < 0)
    {
        printf("channel设置失败 : [%s]\n",snd_strerror(er));
        goto Init_fail;
    }

    //获取采样率并设置
    unsigned int sample_rate = pcm_handle->rate;
    if ((er = snd_pcm_hw_params_set_rate_near(pcm_handle->pcm,hwparams,&sample_rate,0)) < 0)
    {
        printf("采样率设置失败 : [%s]\n",snd_strerror(er));
        goto Init_fail;
    }

    //设置周期数
    if ((er = snd_pcm_hw_params_get_buffer_time_max(hwparams,&buffer_time,0)) < 0)
    {
        printf("snd_pcm_hw_params_get_buffer_time_max fail.\n");
        goto Init_fail;
    }
    if (buffer_time > 500000){
        buffer_time = 500000;
    }
    period_time = buffer_time / 4;
    if ((er = snd_pcm_hw_params_set_buffer_time_near(pcm_handle->pcm,hwparams,&buffer_time,0)) < 0)
    {
        printf("snd_pcm_hw_params_set_buffer_time_near fail.\n");
        goto Init_fail;
    }
     if ((er = snd_pcm_hw_params_set_period_time_near(pcm_handle->pcm,hwparams,&period_time,0)) < 0)
    {
        printf("snd_pcm_hw_params_set_period_time_near fail.\n");
        goto Init_fail;
    }

    //将配置参数写入驱动程序
    if ((er = snd_pcm_hw_params(pcm_handle->pcm,hwparams)) < 0)
    {
        printf("写入配置参数失败 : [%s]\n",snd_strerror(er));
        goto Init_fail;
    }

    //准备音频接口
     if ((er = snd_pcm_prepare(pcm_handle->pcm)) < 0)
    {
        printf("无法使用音频接口 : [%s]\n",snd_strerror(er));
        goto Init_fail;
    }

    //使采集设备处于空闲状态
    //snd_pcm_hw_params_free(hwparams);

    //获取相关参数
    snd_pcm_uframes_t buffer_size;
    snd_pcm_hw_params_get_period_size(hwparams,&(pcm_handle->chunk_size),0);
    snd_pcm_hw_params_get_buffer_size(hwparams,&buffer_size);
    if (pcm_handle->chunk_size == buffer_size)
     {
        printf("Can't use period equal to buffer size (%lu == %lu).\n",pcm_handle->chunk_size,buffer_size);
        goto Init_fail;
    }

    //分配缓冲buff区间
    pcm_handle->bits_per_sample = snd_pcm_format_width(pcm_handle->format)/8;
    pcm_handle->bits_per_frame = pcm_handle->bits_per_sample*params->channel;
    pcm_handle->chunk_bytes = pcm_handle->chunk_size*pcm_handle->bits_per_frame;

    pcm_handle->buffer = (unsigned char *)malloc(pcm_handle->chunk_bytes);
       if (!pcm_handle->buffer) 
       { 
            printf("Error malloc\n"); 
            goto Init_fail; 
        } 
    printf("已初始化录音参数\n");      
    return 0;

Init_fail:
    snd_pcm_close(pcm_handle->pcm);
    return -1;     
}

int finish_to_record_sound()
{

    if(record.buffer) free(record.buffer); 
    if(!init_success) snd_pcm_close(record.pcm);
    init_success = 1;
    printf("停止录音........\n"); 
}

int get_the_record_sound(const char *file)
{ 
    int ret = 0;
    FILE *pcm_file = NULL;
	const char *filename = file;

    if ((pcm_file = fopen(filename,"a")) == NULL)
    {
       printf("无法创建音频文件\n");
       exit(1);
    }

    init_success = Record_Params_Init(&record,&param);

    printf("\n正在录音........\n");
           
    while (!init_success)
    {
        ret = snd_pcm_readi(record.pcm,record.buffer,record.chunk_size);
        if(ret != record.chunk_size) 
        {
            printf("写入数据失败");
           break;
        }
        else if (ret == -EAGAIN)
        {
            snd_pcm_wait(record.pcm, 1000);
            printf("snd_pcm_readi return EAGAIN.\n");
            break;
        }
        else if (ret == -EPIPE)
        {
            snd_pcm_prepare(record.pcm);
            printf("snd_pcm_readi return EPIPE.\n");
            break;
        }
        else if (ret == -ESTRPIPE)
        {
           printf("snd_pcm_readi return ESTRPIPE.\n");
           break;
        }
        else if (ret < 0)
        {
            printf("snd_pcm_readi return fail.\n");
            break;
        }
        
        if (ret > 0)
        {
            //printf("\n正在录音........\n");
           fwrite(record.buffer,record.chunk_size*param.channel,record.bits_per_sample,pcm_file);
        }
        if (record_finish)
        {
           break;
        }
    
    } 
    fclose(pcm_file);
    finish_to_record_sound();
    return 0; 
}
